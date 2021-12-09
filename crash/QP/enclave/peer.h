/*
 * peerype.h
 *
 * definition of the peer type
 */
#ifndef __PEER_HEADER__
#define __PEER_HEADER__

#include <stdint.h>
#include <stdio.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <algorithm>
#include <vector>

class peer {
   public:
    peer() = default;
    peer(SSL_CTX *, bool server);
    ~peer();

    void close();
    int connect(const char *addr, uint16_t server_port);
    int accept(int listen_socket);

    bool want_flush() const;
    bool want_write() const;
    bool want_read() const;
    bool finished_handshake() const;
    bool connected() const;
    std::vector<uint8_t> const &buffer() const;

    int handshake();
    int recv(bool ignoreEOF = false);
    int send();
    int flush();
    void clear_buffer();
    void skip(size_t n_bytes);

    template <typename T, typename S>
    inline int append(T const *buf, S sz) {
        uint8_t const *ptr = reinterpret_cast<uint8_t const *>(buf);
        size_t const size = static_cast<size_t>(sz) * sizeof(T);
        if (sz > 0) {
            std::copy(ptr, ptr + size, std::back_inserter(this->cleartext_buf));
        }
        return 0;
    }

    EVP_PKEY *pubkey() const;  // gives ownership
    uint64_t id() const;
    inline int sock() const { return sock_; };

   private:
    // to allow for reset
    bool set = false;
    bool server;
    SSL_CTX *ctx;

    int sock_;
    SSL *ssl;

    BIO *rbio;  // SSL reads from, we write to
    BIO *wbio;  // SSL writes to, we read from

    // waiting to be written to be encrypted;
    std::vector<uint8_t> cleartext_buf;
    // waiting to be written to socket;
    std::vector<uint8_t> write_buf;
    // waiting to be processed
    std::vector<uint8_t> processed_buf;

    void setup();
    int encrypt(uint8_t const *buf, size_t buf_sz);
    int decrypt(uint8_t const *buf, size_t buf_sz);
};

bool operator==(peer const &lhs, peer const &rhs);
#endif  //__PEER_HEADER__
