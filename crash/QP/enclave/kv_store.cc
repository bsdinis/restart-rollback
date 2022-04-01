#include "kv_store.h"
#include "log.h"

namespace register_sgx {
namespace crash {

int64_t KeyValueStore::get(int64_t key, StoredValue *val) {
    auto key_it = this->m_store.find(key);
    if (key_it == this->m_store.end()) {
        return -1;
    }

    int64_t timestamp = key_it->second.m_timestamp;
    *val = key_it->second.m_val;
    return timestamp;
}

bool KeyValueStore::put(int64_t key, Value const *val, int64_t timestamp,
                        int64_t *current_timestamp) {
    auto key_it = this->m_store.find(key);
    if (key_it == this->m_store.end()) {
        *current_timestamp = timestamp;
        this->m_store.insert({key, TimestampedValue(val, timestamp)});
        return true;
    }

    if (key_it->second.m_timestamp >= timestamp) {
        *current_timestamp = key_it->second.m_timestamp;
        return false;
    }

    *current_timestamp = timestamp;
    key_it->second = TimestampedValue(val, timestamp);
    return true;
}

void KeyValueStore::reset() {
    this->m_store = ::std::unordered_map<int64_t, TimestampedValue>();
}

}  // namespace crash
}  // namespace register_sgx
