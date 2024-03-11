/**
 * kv_store.h
 *
 * local key value store (for fixed size blobs)
 */

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include "byzantine_generated.h"
#include "crypto_helpers.h"
#include "log.h"

namespace register_sgx {
namespace byzantine {

struct TimestampedValue {
    std::array<uint8_t, REGISTER_SIZE> m_val;
    int64_t m_timestamp;
    // Assumption: the signature is a signed hash, which is at most 2048 bits
    // <-> 256 B
    std::array<uint8_t, 256> m_signature;

    TimestampedValue(DataValue const *val, int64_t timestamp,
                     std::vector<uint8_t> const &signature)
        : m_timestamp(timestamp) {
        for (size_t idx = 0; idx < REGISTER_SIZE; ++idx) {
            m_val[idx] = val->data()->Get(idx);
        }

        if (signature.size() > m_signature.size()) {
            KILL("SIGNATURE TOO LARGE: %zu > %zu", signature.size(),
                 m_signature.size());
        }
        std::copy(std::begin(signature), std::end(signature),
                  std::begin(m_signature));
    }
};

struct MACedTimestampedValue {
    TimestampedValue m_ts_val;
    sgx_aes_gcm_128bit_tag_t m_mac;

    MACedTimestampedValue(TimestampedValue &&value, uint8_t *ptr, int64_t key)
        : m_ts_val(std::move(value)) {
        if (ptr == nullptr || register_sgx::byzantine::crypto::encrypt(
                                  ptr, &m_ts_val, sizeof(TimestampedValue),
                                  m_ts_val.m_timestamp, &m_mac) == -1) {
            ERROR("failed to encrypt block at key %ld | pointer %p", key, ptr);
        }
        memcpy(ptr + crypto::padded_size(sizeof(TimestampedValue)), &m_mac,
               sizeof(sgx_aes_gcm_128bit_tag_t));
    }
};

class KeyValueStore {
   private:
    ::std::unordered_map<int64_t, MACedTimestampedValue> m_store;
    void *m_persistent_backed_store = nullptr;
    size_t m_persistent_backed_store_size = 0;

    uint8_t *get_persistent_pointer(int64_t key);

   public:
    void add_backing_store(void *persistent_backed_store,
                           size_t persistent_backed_store_size);

    int64_t get(int64_t key, std::array<uint8_t, REGISTER_SIZE> *val,
                std::vector<uint8_t> &signature);
    int64_t get_timestamp(int64_t key);
    bool put(int64_t key, DataValue const *val, int64_t timestamp,
             std::vector<uint8_t> const &signature, int64_t *current_timestamp);
    void reset();
};

}  // namespace byzantine
}  // namespace register_sgx
