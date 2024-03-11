#include "crypto.h"

#include "log.h"

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <cstdio>
#include <string>

#ifdef INTEL_AES
#include <ippcp.h>
#endif

namespace teems {

std::array<uint8_t, KEY_LEN> gen_key() {
    std::array<uint8_t, KEY_LEN> key;
    if (RAND_bytes(key.data(), KEY_LEN) == 0) {
        ERROR("failed to generate random bytes");
    }

    return key;
}

std::array<uint8_t, IV_LEN> gen_iv() {
    std::array<uint8_t, IV_LEN> iv;
    if (RAND_bytes(iv.data(), IV_LEN) == 0) {
        ERROR("failed to generate random bytes");
    }

    return iv;
}

#if OPENSSL_AES

namespace {
int error_handler(EVP_CIPHER_CTX *ctx);
inline size_t aes_len_padded(size_t input_len) {
    return input_len + (AES_BLOCK_SIZE - input_len % AES_BLOCK_SIZE);
}
}  // namespace

template <size_t N>
std::string array_to_string(std::array<uint8_t, N> const &arr) {
    char str[N * 2 + 1];
    for (size_t idx = 0; idx < N; ++idx) {
        str[idx * 2] = "0123456789abcdef"[arr[idx] / 16];
        str[idx * 2 + 1] = "0123456789abcdef"[arr[idx] % 16];
    }
    str[N * 2] = 0;

    return std::string(str);
}

int aes_encrypt(std::vector<uint8_t> const &plaintext,
                std::array<uint8_t, KEY_LEN> const &key,
                std::array<uint8_t, IV_LEN> const &iv,
                std::array<uint8_t, MAC_LEN> &mac,
                std::vector<uint8_t> &ciphertext) {
    // aes changes the iv, so we use a copy
    std::array<uint8_t, IV_LEN> iv_copy;
    std::copy(std::cbegin(iv), std::cend(iv), std::begin(iv_copy));

    EVP_CIPHER_CTX *openssl_ctx = EVP_CIPHER_CTX_new();
    if (openssl_ctx == nullptr) {
        return error_handler(nullptr);
    }

    if (EVP_EncryptInit_ex(openssl_ctx, EVP_aes_256_gcm(), nullptr, key.data(),
                           iv_copy.data()) != 1) {
        return error_handler(openssl_ctx);
    }

    if (EVP_CIPHER_CTX_ctrl(openssl_ctx, EVP_CTRL_AEAD_SET_IVLEN, IV_LEN,
                            nullptr) != 1) {
        ERROR("failed to set iv size");
        return error_handler(openssl_ctx);
    }

    uint8_t *ciphertext_buffer =
        (uint8_t *)malloc(sizeof(size_t) + aes_len_padded(plaintext.size()));
    size_t full_len = 0;
    int partial_len = 0;
    if (ciphertext_buffer == nullptr) {
        ERROR("failed to allocate ciphertext buffer");
        return -1;
    }

    partial_len = 0;
    size_t plaintext_len = plaintext.size();
    if (EVP_EncryptUpdate(openssl_ctx, ciphertext_buffer + full_len,
                          &partial_len, (uint8_t const *)&plaintext_len,
                          sizeof(size_t)) != 1) {
        free(ciphertext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    partial_len = 0;
    if (EVP_EncryptUpdate(openssl_ctx, ciphertext_buffer + full_len,
                          &partial_len, plaintext.data(),
                          plaintext.size()) != 1) {
        free(ciphertext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    std::array<uint8_t, AES_BLOCK_SIZE> padding_block;
    padding_block.fill(0);

    partial_len = 0;
    if (EVP_EncryptUpdate(
            openssl_ctx, ciphertext_buffer + full_len, &partial_len,
            padding_block.data(),
            aes_len_padded(plaintext.size()) - plaintext.size()) != 1) {
        free(ciphertext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    partial_len = 0;
    if (EVP_EncryptFinal_ex(openssl_ctx, ciphertext_buffer + full_len,
                            &partial_len) != 1) {
        free(ciphertext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    if (EVP_CIPHER_CTX_ctrl(openssl_ctx, EVP_CTRL_AEAD_GET_TAG, MAC_LEN,
                            mac.data()) != 1) {
        free(ciphertext_buffer);
        return error_handler(openssl_ctx);
    }

    ciphertext.clear();
    ciphertext.reserve(full_len);
    std::copy(ciphertext_buffer, ciphertext_buffer + full_len,
              std::back_inserter(ciphertext));
    free(ciphertext_buffer);
    EVP_CIPHER_CTX_free(openssl_ctx);

    return 0;
}

int aes_decrypt(std::vector<uint8_t> const &ciphertext,
                std::array<uint8_t, KEY_LEN> const &key,
                std::array<uint8_t, IV_LEN> const &iv,
                std::array<uint8_t, MAC_LEN> const &mac,
                std::vector<uint8_t> &plaintext) {
    // aes changes the iv, so we use a copy
    std::array<uint8_t, IV_LEN> iv_copy;
    std::copy(std::cbegin(iv), std::cend(iv), std::begin(iv_copy));

    EVP_CIPHER_CTX *openssl_ctx = EVP_CIPHER_CTX_new();
    if (openssl_ctx == nullptr) {
        return error_handler(nullptr);
    }

    if (EVP_DecryptInit_ex(openssl_ctx, EVP_aes_256_gcm(), nullptr, key.data(),
                           iv_copy.data()) != 1) {
        return error_handler(openssl_ctx);
    }

    if (EVP_CIPHER_CTX_ctrl(openssl_ctx, EVP_CTRL_AEAD_SET_IVLEN, IV_LEN,
                            nullptr) != 1) {
        ERROR("failed to set iv size");
        return error_handler(openssl_ctx);
    }

    std::array<uint8_t, IV_LEN> mac_copy;
    std::copy(std::cbegin(mac), std::cend(mac), std::begin(mac_copy));
    if (EVP_CIPHER_CTX_ctrl(openssl_ctx, EVP_CTRL_AEAD_SET_TAG, MAC_LEN,
                            mac_copy.data()) != 1) {
        return error_handler(openssl_ctx);
    }

    uint8_t *plaintext_buffer = (uint8_t *)malloc(ciphertext.size());
    if (plaintext_buffer == nullptr) {
        ERROR("failed to allocate plaintext buffer");
        return -1;
    }

    size_t full_len = 0;
    int partial_len = 0;
    if (EVP_DecryptUpdate(openssl_ctx, plaintext_buffer + full_len,
                          &partial_len, ciphertext.data(),
                          ciphertext.size()) != 1) {
        free(plaintext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    partial_len = 0;
    if (EVP_DecryptFinal_ex(openssl_ctx, plaintext_buffer + full_len,
                            &partial_len) != 1) {
        ERROR("failed final decrypt");
        free(plaintext_buffer);
        return error_handler(openssl_ctx);
    }
    full_len += partial_len;

    size_t plaintext_len = ((size_t *)plaintext_buffer)[0];

    plaintext.clear();
    plaintext.reserve(plaintext_len);
    std::copy(plaintext_buffer + sizeof(size_t),
              plaintext_buffer + sizeof(size_t) + plaintext_len,
              std::back_inserter(plaintext));
    free(plaintext_buffer);
    EVP_CIPHER_CTX_free(openssl_ctx);
    return 0;
}

namespace {

// handle errors
// https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int error_handler(EVP_CIPHER_CTX *ctx) {
    ERR_print_errors_fp(stderr);
    if (ctx != nullptr) {
        EVP_CIPHER_CTX_free(ctx);
    }
    return -1;
}

}  // namespace

#elif INTEL_AES

/* +------------------------------+
 * |                              |
 * |    library implementations   |
 * |                              |
 * +------------------------------+ */

namespace {
size_t aes_len_padded(size_t input_len) { return input_len; }
}  // namespace

int aes_encrypt(std::vector<uint8_t> const &plaintext,
                std::array<uint8_t, KEY_LEN> const &key,
                std::array<uint8_t, IV_LEN> const &iv,
                std::array<uint8_t, MAC_LEN> &mac,
                std::vector<uint8_t> &ciphertext) {
    static_assert(IV_LEN == 16);
    static_assert(KEY_LEN == 16);

    int ctxSize;
    ippsAES_GCMGetSize(&ctxSize);
    Ipp8u arr[ctxSize];
    IppsAES_GCMState *pAES = (IppsAES_GCMState *)arr;
    ippsAES_GCMInit(key.data(), key.size(), pAES, ctxSize);

    // message to be encrypted
    Ipp8u *msg = (Ipp8u *)plaintext.data();
    // and initial counter
    Ipp8u *iv0 = (Ipp8u *)iv.data();
    // encrypted message
    uint8_t *ciphertext_buffer =
        (uint8_t *)malloc(aes_len_padded(plaintext.size()));
    if (ciphertext_buffer == nullptr) {
        ERROR("failed to allocate ciphertext buffer");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        return -1;
    }
    Ipp8u *ctext = (Ipp8u *)ciphertext_buffer;

    if (ippsAES_GCMStart(iv0, 16, nullptr, 0, pAES) != ippStsNoErr) {
        ERROR("IPP: failed to start GCM");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(ciphertext_buffer);
        return -1;
    }
    if (ippsAES_GCMEncrypt(msg, ctext, plaintext.size(), pAES) != ippStsNoErr) {
        ERROR("IPP: failed to GCM encrypt");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(ciphertext_buffer);
        return -1;
    }
    if (ippsAES_GCMGetTag((Ipp8u *)mac.data(), MAC_LEN, pAES) != ippStsNoErr) {
        ERROR("IPP: failed to get GCM tag");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(ciphertext_buffer);
        return -1;
    }

    // remove secret and release resource
    ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);

    ciphertext.clear();
    ciphertext.reserve(aes_len_padded(plaintext.size()));
    std::copy(ciphertext_buffer,
              ciphertext_buffer + aes_len_padded(plaintext.size()),
              std::back_inserter(ciphertext));
    free(ciphertext_buffer);

    return 0;
}

/*
 * function: aes_decrypt
 *   ciphertext: content to decipher
 *   clen: plaintext len
 *   key: KEY_LEN sized len
 *   iv: IV_LEN sized initialization vector
 *   decrypted: allocd memory to hold ciphertext
 *
 *   decrypted must have the size of ciphertext, since that
 *   is the maximum it can hold
 *
 * decrypt plaintext using AES_CBC
 * return: decrypted len
 */

int aes_decrypt(std::vector<uint8_t> const &ciphertext,
                std::array<uint8_t, KEY_LEN> const &key,
                std::array<uint8_t, IV_LEN> const &iv,
                std::array<uint8_t, MAC_LEN> const &mac,
                std::vector<uint8_t> &plaintext) {
    static_assert(IV_LEN == 16);
    static_assert(KEY_LEN == 16);

    int ctxSize;
    ippsAES_GCMGetSize(&ctxSize);
    Ipp8u arr[ctxSize];
    IppsAES_GCMState *pAES = (IppsAES_GCMState *)arr;
    ippsAES_GCMInit(key.data(), key.size(), pAES, ctxSize);

    // message to be decrypted
    Ipp8u *ctext = (Ipp8u *)ciphertext.data();
    // and initial counter
    Ipp8u *iv0 = (Ipp8u *)iv.data();
    // decrypted message
    uint8_t *plaintext_buffer = (uint8_t *)malloc(ciphertext.size());
    if (plaintext_buffer == nullptr) {
        ERROR("failed to allocate plaintext buffer");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        return -1;
    }
    Ipp8u *rtext = (Ipp8u *)plaintext_buffer;

    if (ippsAES_GCMStart(iv0, 16, nullptr, 0, pAES) != ippStsNoErr) {
        ERROR("IPP: failed to start GCM");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(plaintext_buffer);
        return -1;
    }
    if (ippsAES_GCMDecrypt(ctext, rtext, clen, pAES) != ippStsNoErr) {
        ERROR("IPP: failed to GCM decrypt");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(plaintext_buffer);
        return -1;
    }
    std::array<uint8_t, MAC_LEN> new_mac;
    if (ippsAES_GCMGetTag((Ipp8u *)new_mac.data(), MAC_LEN, pAES) !=
        ippStsNoErr) {
        ERROR("IPP: failed to get GCM tag");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(plaintext_buffer);
        return -1;
    }

    if (new_mac != mac) {
        ERROR("IPP: mac check failed");
        ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);
        free(plaintext_buffer);
        return -1;
    }

    ippsAES_GCMInit(0, KEY_LEN, pAES, ctxSize);

    plaintext.clear();
    plaintext.reserve(ciphertext.size());
    std::copy(plaintext_buffer, plaintext_buffer + ciphertext.size(),
              std::back_inserter(plaintext));
    free(plaintext_buffer);

    return clen;
}

#else
#error "no crypto library defined"
#endif

}  // namespace teems
