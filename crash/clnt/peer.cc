/**
 * implementation
 */

#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

#include "log.h"
#include "peer.h"
#include "ssl_util.h"

namespace {
size_t constexpr DEFAULT_BUF_SIZE = (1 << 10);

inline bool ssl_status_want_io(int status) {
    return status == SSL_ERROR_WANT_WRITE || status == SSL_ERROR_WANT_READ;
}
inline bool ssl_status_ok(int status) { return status == SSL_ERROR_NONE; }
inline bool ssl_status_fail(int status) {
    return !ssl_status_ok(status) && !ssl_status_want_io(status);
}
}  // namespace

void peer::setup() {
    if (this->set) return;
    this->rbio = BIO_new(BIO_s_mem());
    this->wbio = BIO_new(BIO_s_mem());
    this->ssl = SSL_new(this->ctx);

    if (this->server)
        SSL_set_accept_state(this->ssl);
    else
        SSL_set_connect_state(this->ssl);

    SSL_set_bio(this->ssl, this->rbio, this->wbio);
    this->set = true;
}

peer::peer(SSL_CTX *ctx, bool server) {
    this->sock_ = -1;
    this->server = server;
    this->ctx = ctx;

    this->setup();
}

peer::~peer() { this->close(); }

static int connect_block(char const *addr, uint16_t server_port) {
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock < 0) {
        ERROR("failed to open socket");
        return -1;
    }

    {
        int one = 1;
        if (setsockopt(sock, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1) {
            KILL("setsockopt failed: %s", strerror(errno));
        }
    }

    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(server_port);
    errno = 0;
    if (inet_pton(AF_INET, addr, &(s_addr.sin_addr)) == -1) {
        ERROR("inet_pton: %s", strerror(errno));
        return -1;
    }

    errno = 0;
    while (connect(sock, (struct sockaddr *)&s_addr,
                   sizeof(struct sockaddr_in)) == -1 &&
           (errno == EINPROGRESS || errno == EALREADY))
        ;

    if (errno != 0 && errno != EINPROGRESS && errno != EALREADY) {
        ERROR("failed to connect to %s:%d - %s", addr, server_port,
              strerror(errno));
        return -1;
    }
    errno = 0;

    return sock;
}

int peer::connect(char const *addr, uint16_t server_port) {
    this->setup();
    this->sock_ = connect_block(addr, server_port);
    if (this->sock_ == -1) {
        ERROR("failed to connect: connect()");
        return -1;
    }

    return 0;
}

int peer::accept(int listen_socket) {
    this->setup();
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    this->sock_ = ::accept(listen_socket, (struct sockaddr *)&addr, &len);
    if (this->sock_ == -1) {
        ERROR("failed to accept: %s", strerror(errno));
        return -1;
    }
    {
        int one = 1;
        if (setsockopt(this->sock_, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) ==
            -1) {
            KILL("setsockopt failed: %s", strerror(errno));
        }
    }
    return 0;
}

void peer::close() {
    if (!this->connected()) return;
    if (this->sock_ != -1) ::shutdown(this->sock_, SHUT_RDWR);
    this->sock_ = -1;

    this->cleartext_buf.clear();
    this->write_buf.clear();
    this->processed_buf.clear();

    // SSL object has garbage, needs to be reset to allow for
    // another connection
    if (this->ssl) SSL_free(this->ssl);
    this->ssl = nullptr;
    this->set = false;
}

bool peer::want_flush() const {
    return !this->want_write() && this->cleartext_buf.size() > 0;
}
bool peer::want_write() const { return this->write_buf.size() > 0; }
bool peer::want_read() const { return this->processed_buf.size() > 0; }
bool peer::connected() const { return this->sock_ > 0; }

bool operator==(peer const &lhs, peer const &rhs) {
    return lhs.sock() == rhs.sock();
}

bool peer::finished_handshake() const {
    return this->connected() && SSL_is_init_finished(this->ssl);
}

std::vector<uint8_t> const &peer::buffer() const { return this->processed_buf; }
void peer::clear_buffer() { this->processed_buf.clear(); }
void peer::skip(size_t n_bytes) {
    this->processed_buf.erase(begin(this->processed_buf),
                              begin(this->processed_buf) + n_bytes);
}

int peer::handshake() {
    int ret = SSL_do_handshake(this->ssl);
    int const status = SSL_get_error(this->ssl, ret);

    if (ssl_status_fail(status)) {
        // this is yet to be solved
        // see issue #1 on https://gitlab.mpi-sws.org/sybil/epidemics
        ERROR("SSL has a failure status");
    } else if (ssl_status_want_io(status)) {
        // ssl may want to read stuff
        uint8_t buf[DEFAULT_BUF_SIZE];
        do {
            ret = BIO_read(this->wbio, buf, sizeof(buf));
            if (ret > 0) {
                try {
                    std::copy(buf, buf + ret,
                              std::back_inserter(this->write_buf));
                } catch (std::bad_alloc const &) {
                    ERROR("Failed to allocate memory in handshake");
                    return -1;
                }
            } else if (!BIO_should_retry(this->wbio)) {
                ERROR("OpenSSL says give-up");
                return -1;
            }
        } while (ret > 0);
    }

    return 0;
}

/* Read encrypted bytes from socket. */
int peer::recv() {
    uint8_t buf[DEFAULT_BUF_SIZE];
    int nbytes = read(this->sock(), buf, DEFAULT_BUF_SIZE);

    if (nbytes < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ERROR("read(): failed");
            return -1;
        }
        errno = 0;
        return 0;
    } else if (nbytes == 0) {
        INFO("closing socket %d: EOF", this->sock_);
        this->close();
        return 0;
    }

    return this->decrypt(buf, static_cast<size_t>(nbytes));
}

int peer::send() {
    size_t const max_size =
        std::min(this->write_buf.size(), static_cast<size_t>(0xd8000));
    int nwritten = write(this->sock(), this->write_buf.data(), max_size);

    if (nwritten > 0) {
        this->write_buf.erase(std::begin(this->write_buf),
                              std::begin(this->write_buf) + nwritten);
        return 0;
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        ERROR("write(): failed");
        return -1;
    }

    errno = 0;
    return 0;
}

int peer::flush() {
    if (this->encrypt(this->cleartext_buf.data(), this->cleartext_buf.size()) ==
        -1) {
        return -1;
    }
    this->cleartext_buf.clear();
    return 0;
}

// get a public key
EVP_PKEY *peer::pubkey() const {
    if (this->ssl) return NULL;
    X509 *cert = SSL_get_peer_certificate(this->ssl);
    if (cert == NULL) {
        ERROR("Failed to get the certificate");
        return NULL;
    }

    EVP_PKEY *key = X509_get_pubkey(cert);  // no ownership
    X509_free(cert);
    return key;
}

// get id
uint64_t peer::id() const {
    EVP_PKEY *key = this->pubkey();
    if (!key) return 0;

    uint64_t uid = 0;
    uint8_t *buffer = NULL;
    int len = i2d_PublicKey(key, &buffer);

    for (int i = 0; i < len / 8; i++) {
        uint64_t block = 0;
        for (int j = 0; j < 8; j++) {
            block <<= 8;
            block |= buffer[8 * i + j];
        }
        uid ^= block;
    }

    free(buffer);
    EVP_PKEY_free(key);
    return uid;
}

int peer::encrypt(uint8_t const *buf, size_t buf_sz) {
    if (!this->ssl || !SSL_is_init_finished(this->ssl)) return 0;

    int written = 0;
    while (written < static_cast<ssize_t>(buf_sz)) {
        int ret =
            SSL_write(this->ssl, buf + written, (int)buf_sz - (int)written);
        int status = SSL_get_error(this->ssl, ret);

        if (ret > 0) {
            written += ret;

            /* take the output of the SSL object
             * and queue it for socket write */
            do {
                uint8_t enc_buf[DEFAULT_BUF_SIZE];
                ret = BIO_read(this->wbio, enc_buf, sizeof(enc_buf));
                if (ret > 0) {
                    try {
                        std::copy(enc_buf, enc_buf + ret,
                                  std::back_inserter(this->write_buf));
                    } catch (std::bad_alloc const &) {
                        ERROR("Failed to allocate memory in SSL encrypt");
                        return -1;
                    }
                } else if (!BIO_should_retry(this->wbio)) {
                    ERROR("OpenSSL says give-up");
                    return -1;
                }
            } while (ret > 0);
        }

        if (ssl_status_fail(status)) return -1;

        if (ret == 0) break;
    }
    return 0;
}

int peer::decrypt(uint8_t const *buf, size_t buf_sz) {
    while (buf_sz > 0) {
        int ret = BIO_write(this->rbio, buf, (int)buf_sz);

        if (ret <= 0) {
            ERROR("failed BIO_write");
            return -1;  // bio failure is irrecoverable
        }

        buf += ret;
        buf_sz -= ret;

        if (!SSL_is_init_finished(this->ssl)) {
            if (this->handshake() == -1) {
                ERROR("failed handshake");
                return -1;
            }
            if (!SSL_is_init_finished(this->ssl)) return 0;
        }

        // read cleartext
        do {
            uint8_t dec_buf[DEFAULT_BUF_SIZE];
            memset(dec_buf, 0, sizeof(dec_buf));
            ret = SSL_read(this->ssl, dec_buf, sizeof(dec_buf));
            if (ret > 0) {
                try {
                    std::copy(dec_buf, dec_buf + ret,
                              std::back_inserter(this->processed_buf));
                } catch (std::bad_alloc const &) {
                    ERROR("Failed to allocate in SSL decrypt");
                    return -1;
                }
            }
        } while (ret > 0);

        int status = SSL_get_error(this->ssl, ret);

        // may have renegotiation
        if (ssl_status_want_io(status)) do {
                uint8_t enc_buf[DEFAULT_BUF_SIZE];
                memset(enc_buf, 0, sizeof(enc_buf));
                ret = BIO_read(this->wbio, enc_buf, sizeof(enc_buf));
                if (ret > 0) {
                    try {
                        std::copy(enc_buf, enc_buf + ret,
                                  std::back_inserter(this->write_buf));
                    } catch (std::bad_alloc const &) {
                        ERROR("Failed to allocate memory in SSL decrypt");
                        return -1;
                    }
                } else if (!BIO_should_retry(this->wbio)) {
                    ERROR("failed BIO read");
                    return -1;
                }
            } while (ret > 0);

        if (ssl_status_fail(status)) {
            ERROR("ssl status is fail");
            return -1;
        }
    }

    return 0;
}
