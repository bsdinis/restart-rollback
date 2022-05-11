/**
 * ssl_util.c
 */

#include "ssl_util.h"
#include <unistd.h>
#include "enclave_t.h"
#include "log.h"

namespace {
EVP_PKEY *generatePrivateKey();
X509 *generateCertificate(EVP_PKEY *pkey);
int init_ssl_ctx(SSL_CTX **, bool server);
int dummy_verify_cb(int ok, X509_STORE_CTX *ctx);
}  // anonymous namespace

namespace teems {
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

}  // namespace ssl
}  // namespace teems

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
