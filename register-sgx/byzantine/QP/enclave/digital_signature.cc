#include "digital_signature.h"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>

extern EVP_PKEY* g_rsa_pkey;

namespace register_sgx {
namespace byzantine {

int sign_value(int64_t key, int64_t timestamp,
               std::array<uint8_t, REGISTER_SIZE> const& value,
               std::vector<uint8_t>& signature) {
    EVP_MD_CTX* rsa_sign_ctx = EVP_MD_CTX_create();
    if (EVP_DigestSignInit(rsa_sign_ctx, NULL, EVP_sha256(), NULL,
                           g_rsa_pkey) <= 0) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }
    if (EVP_DigestSignUpdate(rsa_sign_ctx, &key, sizeof(int64_t)) <= 0) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }
    if (EVP_DigestSignUpdate(rsa_sign_ctx, &timestamp, sizeof(int64_t)) <= 0) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }
    if (EVP_DigestSignUpdate(rsa_sign_ctx, value.data(), value.size()) <= 0) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }

    size_t signature_size = 0;
    if (EVP_DigestSignFinal(rsa_sign_ctx, NULL, &signature_size) <= 0) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }

    uint8_t* signature_ptr = (uint8_t*)malloc(signature_size);
    if (signature_ptr == nullptr) {
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }

    if (EVP_DigestSignFinal(rsa_sign_ctx, signature_ptr, &signature_size) <=
        0) {
        free(signature_ptr);
        EVP_MD_CTX_cleanup(rsa_sign_ctx);
        return -1;
    }

    signature.reserve(signature_size);
    std::copy(signature_ptr, signature_ptr + signature_size,
              std::back_inserter(signature));
    free(signature_ptr);

    EVP_MD_CTX_cleanup(rsa_sign_ctx);
    return 0;
}

int verify_value(SignedValue const* value, bool* authentic) {
    *authentic = false;

    EVP_MD_CTX* rsa_verify_ctx = EVP_MD_CTX_create();

    if (EVP_DigestVerifyInit(rsa_verify_ctx, NULL, EVP_sha256(), NULL,
                             g_rsa_pkey) <= 0) {
        return -1;
    }

    int64_t key = value->value()->key();
    int64_t timestamp = value->value()->timestamp();
    if (EVP_DigestVerifyUpdate(rsa_verify_ctx, &key, sizeof(int64_t)) <= 0) {
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return -1;
    }
    if (EVP_DigestVerifyUpdate(rsa_verify_ctx, &timestamp, sizeof(int64_t)) <=
        0) {
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return -1;
    }
    if (EVP_DigestVerifyUpdate(
            rsa_verify_ctx, value->value()->data_value()->data()->Data(),
            value->value()->data_value()->data()->size()) <= 0) {
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return -1;
    }

    int AuthStatus = EVP_DigestVerifyFinal(
        rsa_verify_ctx, value->signature()->data(), value->signature()->size());
    if (AuthStatus == 1) {
        *authentic = true;
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return 0;
    } else if (AuthStatus == 0) {
        *authentic = false;
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return 0;
    } else {
        *authentic = false;
        EVP_MD_CTX_cleanup(rsa_verify_ctx);
        return -1;
    }
}

}  // namespace byzantine
}  // namespace register_sgx
