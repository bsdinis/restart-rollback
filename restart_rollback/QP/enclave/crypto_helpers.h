/**
 * crypto_helpers.h
 *
 * help encrypt stuff for the kernel
 */

#ifndef __CRYPTO_HELPERS_HEADER__
#define __CRYPTO_HELPERS_HEADER__

#include <sgx_tcrypto.h>
#include <stdint.h>
#include <stdio.h>

#define AES_BLOCK_SIZE (16)

namespace paxos_sgx {
namespace restart_rollback {
namespace crypto {

int encrypt(uint8_t* enc_val, void const* val, size_t size, uint64_t counter,
            sgx_aes_gcm_128bit_tag_t* mac);
int decrypt(uint8_t const* enc_val, void* val, size_t size, uint64_t counter,
            sgx_aes_gcm_128bit_tag_t const* mac);
constexpr size_t padded_size(size_t size) {
    return size + (AES_BLOCK_SIZE - size % AES_BLOCK_SIZE);
}

}  // namespace crypto
}  // namespace restart_rollback
}  // namespace paxos_sgx

#endif  //__CRYPTO_HELPERS_HEADER__
