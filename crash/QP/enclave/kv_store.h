/**
 * kv_store.h
 *
 * local key value store (for fixed size blobs)
 */

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include "crash_generated.h"
#include "crypto_helpers.h"
#include "log.h"

namespace register_sgx {
namespace crash {

constexpr int64_t VALUE_SIZE_B = 2048;
using StoredValue = std::array<uint8_t, VALUE_SIZE_B>;

struct TimestampedValue {
    StoredValue m_val;
    int64_t m_timestamp;

    TimestampedValue(Value const *val, int64_t timestamp)
        : m_timestamp(timestamp) {
        for (size_t idx = 0; idx < VALUE_SIZE_B; ++idx) {
            m_val[idx] = val->data()->Get(idx);
        }
    }
};

struct MACedTimestampedValue {
    TimestampedValue m_ts_val;
    sgx_aes_gcm_128bit_tag_t m_mac;

    MACedTimestampedValue(TimestampedValue &&value, uint8_t *ptr, int64_t key)
        : m_ts_val(std::move(value)) {
        if (ptr == nullptr || register_sgx::crash::crypto::encrypt(
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

    int64_t get(int64_t key, StoredValue *val);
    bool put(int64_t key, Value const *val, int64_t timestamp,
             int64_t *current_timestamp);
    void reset();
};

}  // namespace crash
}  // namespace register_sgx
