/**
 * ssl_util.c
 */

#include "ssl_util.h"
#include <openssl/pem.h>
#include <unistd.h>
#include "enclave_t.h"
#include "log.h"

namespace {
EVP_PKEY *generatePrivateKey();
X509 *generateCertificate(EVP_PKEY *pkey);
int init_ssl_ctx(SSL_CTX **, bool server);
int dummy_verify_cb(int ok, X509_STORE_CTX *ctx);

// HACK
char const *client_key = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA6WitbyfYt5lrEeSDsreiIm6l7dKE4/MY1jEbwWFj5GpTn6KI
c+yK/d1FFn/jcVMbkWMiCcnTbGFrMizS1p/3tbjkrdvLxaXk+AEDvVuYntDl314b
AiR/jKFtdAF2O6tBsf41ZsoXOByewnt3YyaJGuKXuW887h5aGrrH7SP1Rb59hUf+
3JNR0D3DhXl67uv3zUI27EElfb/0StcoqmS3oT77GdKBQirmvm++1sWUe5kgs6Mu
pSX1apMqxQiA7On1D9df2EOjnZ2gBdzFJ3hG+EqF9l5Z+/YZDXP3aBGFsqTx33Xt
v37p5XtkFo/LmHJRLfGeIwCOnNwLMrnHG3Zb5QIDAQABAoIBADTT1VUwLpO3bI1b
LBeaCl9Crdl66UrybW+lI8QL9+Dt08lwrvOV2QuGNinWfjLa/YVVa5HNlUWVcv6j
OIENKbJ+i3tyINe2KXGcQS1tesiPPUfl87Tzb87zhmNYJVtx8meHwqxmFwVDLQeV
jDrrsOJd4Ls+mwrF4dzk43Ufy1aiTOVkMVxk5wmrNx28zzhMe0OKby3DaP9DTZCI
dTyQI8Nf+NzHVsxQbGiBMEcS9pD1uV+hxO8fti1BODT4MOcIFYztJSGKmcyyRi+T
TD8diB4KpnEkjNBr1ekwrPCYiKGoFej74Vv0FrtuZl3IEbAoSLjLEVVGPHu7JGRt
6yt2l00CgYEA/snjkkoC0xt27GaFeVOXvaHtNt74/LhVfej7kvlZbIvcveG2TPE8
2lQRvpkD86cUe24c6TefYVix7hZI44KDAD3R278/0c1hH4hFhF/WfxjElfRc23s1
0Gw9OlUSW5hkvQRtPwkVA2NFS6vGvPwCW4T7mKDol5d6RjaViVNWuosCgYEA6oTE
P9PZmIhh91x3AFy8h6BeiCsy0HI5I8RjT+EaebU9TRoB3+0v9vbKt80sRtOmcynA
qk61IzKhVkHE/A8jRkO6VeslR05qDMpDbhgl7W+I5c+7JeEhYEYcltdb/uaDhfiI
aLazCA0BIa6WdqBE73riNMqa9qi+OQpkWPUwwU8CgYAwMSemR/F41L0NweaFKFaP
O82Oj2YxBvzEsAjFp/xOF7ZNBCBjNU35TeV9U5v3viaVoiI9ypOO6iPdkMGApvL4
XLkRA3+iw2IXgMMFYstDhv/IDUxzBz8KiS6blfO+93g2eFvhjGo5FdHJmlgg7tnT
Tbf5CpZz9ebe2Pw32VeSOQKBgGFB/E1oI0aR0OcP7r40t28bv/iCyckOhcniiTk4
Zo1w72JJVduy9wBZqwp8yU76OX5rI16903ssZSJrljDeakeakO1O3p/q0ipJu5st
4pTMe56q/rcf3YKasQKRf7J/kK3/uzqKAkdldUGtsPKzVsNmV+eC2q9Oncz0FVeb
H8QbAoGBANVpVZbaP6lLj2EXiSP8uejxsQteWY0J4P93pb2qylKIF2MqtaqfuSeM
F4NNqkJVFonX7tj4RgZ3xqRxwRlDr6Ak6caZ0rSFxaoj/SZS/BSbzw/ULZ99taJd
M0YhaTzFGI2P+utt6CPc9xv3Q7e7OMuQX/1lumBWL77hgxhYmPqN
-----END RSA PRIVATE KEY-----)";
}  // anonymous namespace

namespace register_sgx {
namespace byzantine {
namespace ssl {

int init_client_ssl_ctx(SSL_CTX **ctx_ptr) {
    return init_ssl_ctx(ctx_ptr, false);
}

int init_server_ssl_ctx(SSL_CTX **ctx_ptr) {
    return init_ssl_ctx(ctx_ptr, true);
}

int close_ssl_ctx(SSL_CTX *ctx) {
    ERR_free_strings();
    SSL_CTX_free(ctx);
    return 0;
}

int load_certificates(SSL_CTX *ctx, const char *const cert,
                      const char *const key) {
    (void)cert;  // HACK
    (void)key;   // HACK
    EVP_PKEY *pkey = generatePrivateKey();
    X509 *x509 = generateCertificate(pkey);

    int ret = 0;
    if (SSL_CTX_use_certificate(ctx, x509) <= 0) {
        ERROR("Failed to set certificate file");
        ret = -1;
    } else if (SSL_CTX_use_PrivateKey(ctx, pkey) <= 0) {
        ERROR("Failed to set key file");
        ret = -1;
    } else if (!SSL_CTX_check_private_key(ctx)) {
        ERROR("Failed to validate private key");
        ret = -1;
    }

    return ret;
}

int load_key(RSA **priv_key, const char *const key_file) {
    // HACK
    (void)key_file;
    BIO *key_bio = BIO_new_mem_buf((void *)client_key, -1);

    if (key_bio == nullptr) {
        ERROR("failed to create BIO to read RSA key");
        return -1;
    }

    *priv_key = PEM_read_bio_RSAPrivateKey(key_bio, priv_key, nullptr, nullptr);
    if (*priv_key == nullptr) {
        ERROR("failed to read RSA key");
        return -1;
    }

    BIO_free(key_bio);

    return 0;
}

}  // namespace ssl
}  // namespace byzantine
}  // namespace register_sgx

namespace {
EVP_PKEY *generatePrivateKey() {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048);
    EVP_PKEY_keygen(pctx, &pkey);
    return pkey;
}

X509 *generateCertificate(EVP_PKEY *pkey) {
    X509 *x509 = X509_new();
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), (long)60 * 60 * 24 * 365);
    X509_set_pubkey(x509, pkey);

    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                               (const unsigned char *)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (const unsigned char *)"localhost", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_md5());
    return x509;
}

int init_ssl_ctx(SSL_CTX **ctx_ptr, bool server) {
    SSL_library_init();

    // add error strings
    SSL_load_error_strings();
    ERR_load_crypto_strings();

    (*ctx_ptr) =
        SSL_CTX_new(server ? TLSv1_2_server_method() : TLSv1_2_client_method());

    if ((*ctx_ptr) == NULL) {
        ERROR("Failed to create CTX\n");
        return -1;
    }

    int mode =
        server ? SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE : SSL_VERIFY_NONE;
    SSL_CTX_set_verify((*ctx_ptr), mode, server ? dummy_verify_cb : NULL);
    SSL_CTX_set_verify_depth((*ctx_ptr), 1);

    // disable SSLv23
    SSL_CTX_set_options(*ctx_ptr,
                        SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    return 0;
}

int dummy_verify_cb(int ok, X509_STORE_CTX *ctx) {
    /*
     * ATTENTION: this verification is dummy
     * should not be used in production
     */
    (void)ok;
    (void)ctx;
    return 1;
}
}  // anonymous namespace
