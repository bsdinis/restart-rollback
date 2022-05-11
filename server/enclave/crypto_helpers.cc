/**
 * crypto_helpers.cc
 *
 * help encrypt stuff for the kernel
 *
 * strawman
 */

#include "crypto_helpers.h"
#include "log.h"

#include <sgx_tcrypto.h>
#include <string.h>

#define KEY_LEN (16)
#define IV_LEN (16)
#define MAC_LEN (16)

static uint8_t const strawman_key[KEY_LEN] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

namespace teems {
namespace crypto {

int encrypt(uint8_t *const encrypted, void const *val, size_t size,
            uint64_t counter, sgx_aes_gcm_128bit_tag_t *mac) {
    uint8_t strawman_iv[IV_LEN] = {0};
    ((uint64_t *)(strawman_iv))[0] = counter;
    ((uint64_t *)(strawman_iv))[1] = counter;

    sgx_status_t ret = sgx_rijndael128GCM_encrypt(
        (const sgx_aes_gcm_128bit_key_t *)&strawman_key, (const uint8_t *)val,
        size, encrypted, (uint8_t const *)&strawman_iv, 12, NULL, 0, mac);

    if (ret != SGX_SUCCESS) {
        LOG("failed to encrypt");
        memset(encrypted, 0, padded_size(size));
        return -1;
    }

    return 0;
}

int decrypt(uint8_t const *enc_val, void *val, size_t size, uint64_t counter,
            sgx_aes_gcm_128bit_tag_t const *mac) {
    sgx_status_t res;

    uint8_t strawman_iv[IV_LEN] = {0};
    ((uint64_t *)(strawman_iv))[0] = counter;
    ((uint64_t *)(strawman_iv))[1] = counter;

    sgx_status_t ret = sgx_rijndael128GCM_decrypt(
        (const sgx_aes_gcm_128bit_key_t *)&strawman_key,
        (const uint8_t *)enc_val, size, (uint8_t *)val,
        (uint8_t const *)&strawman_iv, 12, NULL, 0, mac);

    // TODO: set a known field in sgx_aes_ctr_encrypt and check it's validity
    if (res != SGX_SUCCESS) {
        LOG("failed to decrypt");
        return -1;
    }

    return 0;
}

}  // namespace crypto
}  // namespace teems
