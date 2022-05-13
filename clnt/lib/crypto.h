#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

namespace teems {

#ifdef INTEL_AES
constexpr size_t AES_TYPE = 128;
#else
constexpr size_t AES_TYPE = 256;
#endif

constexpr size_t AES_BLOCK_SIZE = 16;
constexpr size_t KEY_LEN = AES_TYPE / 8;
constexpr size_t IV_LEN = AES_BLOCK_SIZE;
constexpr size_t MAC_LEN = 16;


int aes_encrypt(std::vector<uint8_t> const &plaintext,
    std::array<uint8_t, KEY_LEN> const& key,
    std::array<uint8_t, IV_LEN> const& iv,
    std::array<uint8_t, MAC_LEN> & mac,
    std::vector<uint8_t> &ciphertext
    );

int aes_decrypt(std::vector<uint8_t> const &ciphertext,
    std::array<uint8_t, KEY_LEN> const& key,
    std::array<uint8_t, IV_LEN> const& iv,
    std::array<uint8_t, MAC_LEN> const& mac,
    std::vector<uint8_t> &plaintext);

std::array<uint8_t, KEY_LEN> gen_key();
std::array<uint8_t, IV_LEN> gen_iv();

} // namespace teems
