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

namespace register_sgx {
namespace crash {

constexpr int64_t VALUE_SIZE_B = 2048;
using StoredValue = std::array<uint8_t, VALUE_SIZE_B>;

struct TimestampedValue {
    StoredValue m_val;
    int64_t m_timestamp;

    // TODO
    TimestampedValue(Value const *val, int64_t timestamp)
        : m_timestamp(timestamp) {
        for (size_t idx = 0; idx < VALUE_SIZE_B; ++idx) {
            m_val[idx] = val->data()->Get(idx);
        }
    }
};

class KeyValueStore {
   private:
    ::std::unordered_map<int64_t, TimestampedValue> m_store;

   public:
    int64_t get(int64_t key, StoredValue *val);
    bool put(int64_t key, Value const *val, int64_t timestamp,
             int64_t *current_timestamp);
    void reset();
};

}  // namespace crash
}  // namespace register_sgx
