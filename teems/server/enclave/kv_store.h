/**
 * kv_store.h
 *
 * local key value store (for fixed size blobs)
 */

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include "crypto_helpers.h"
#include "log.h"
#include "policy.h"
#include "teems_generated.h"

namespace teems {

struct TimestampedValue {
    std::array<uint8_t, REGISTER_SIZE> m_val;
    int64_t m_timestamp;
    bool m_stable;

    TimestampedValue(Value const *val, int64_t timestamp, bool stable)
        : m_timestamp(timestamp) {
        for (size_t idx = 0; idx < REGISTER_SIZE; ++idx) {
            m_val[idx] = val->data()->Get(idx);
        }
    }
};

struct MACedTimestampedValue {
    TimestampedValue m_ts_val;
    sgx_aes_gcm_128bit_tag_t m_mac;

    MACedTimestampedValue(TimestampedValue &&value, uint8_t *ptr, int64_t key)
        : m_ts_val(std::move(value)) {
        if (ptr == nullptr ||
            teems::crypto::encrypt(ptr, &m_ts_val, sizeof(TimestampedValue),
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
    ::std::unordered_map<int64_t, std::tuple<int64_t, ServerPolicy>>
        m_policy_store;

    void *m_persistent_backed_store = nullptr;
    size_t m_persistent_backed_store_size = 0;

    uint8_t *get_persistent_pointer(int64_t key);

   public:
    void add_backing_store(void *persistent_backed_store,
                           size_t persistent_backed_store_size);

    bool get(int64_t key, bool *stable, bool *suspicious,
             int64_t *policy_version, int64_t *timestamp, ServerPolicy *policy,
             std::array<uint8_t, REGISTER_SIZE> *val);

    bool get_timestamp(int64_t key, bool *stable, bool *suspicious,
                       int64_t *policy_version, int64_t *timestamp,
                       ServerPolicy *policy);

    bool put(int64_t key, Value const *val, ServerPolicy const &policy,
             int64_t policy_version, int64_t timestamp,
             int64_t *current_policy_version, int64_t *current_timestamp);

    bool get_policy(int64_t key, ServerPolicy *policy,
                    int64_t *current_policy_version);
    bool change_policy(int64_t key, ServerPolicy policy,
                       int64_t *current_policy_version);

    void stabilize(int64_t key, int64_t policy_version, int64_t timestamp);
    void reset();
};

}  // namespace teems
