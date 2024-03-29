#include "kv_store.h"

#include "crypto_helpers.h"
#include "log.h"
#include "setup.h"

namespace register_sgx {
namespace restart_rollback {

uint8_t *KeyValueStore::get_persistent_pointer(int64_t key) {
    uint8_t *persistent_ptr =
        ((uint8_t *)m_persistent_backed_store) +
        key * crypto::padded_size(sizeof(TimestampedValue));
    if (persistent_ptr < m_persistent_backed_store ||
        persistent_ptr >= ((uint8_t *)m_persistent_backed_store) +
                              m_persistent_backed_store_size) {
        ERROR(
            "trying to write outside persistently backed memory region [%p ; "
            "%p) (write at %p, key %ld)",
            m_persistent_backed_store,
            ((uint8_t *)m_persistent_backed_store) +
                m_persistent_backed_store_size,
            persistent_ptr, key);
        return nullptr;
    }

    return persistent_ptr;
}

void KeyValueStore::add_backing_store(void *persistent_backed_store,
                                      size_t persistent_backed_store_size) {
    m_persistent_backed_store = persistent_backed_store;
    m_persistent_backed_store_size = persistent_backed_store_size;
    m_store.reserve(m_persistent_backed_store_size /
                    (crypto::padded_size(sizeof(TimestampedValue)) +
                     sizeof(sgx_aes_gcm_128bit_tag_t)));
}

int64_t KeyValueStore::get(int64_t key, bool *stable, bool *suspicious,
                           std::array<uint8_t, REGISTER_SIZE> *val) {
    assert(stable != nullptr);
    assert(suspicious != nullptr);
    *suspicious = setup::is_suspicious();

    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        *stable = true;
        return -1;
    }

    int64_t timestamp = key_it->second.m_ts_val.m_timestamp;
    *stable = key_it->second.m_ts_val.m_stable;
    *val = key_it->second.m_ts_val.m_val;
    return timestamp;
}

int64_t KeyValueStore::get_timestamp(int64_t key, bool *suspicious) {
    assert(suspicious != nullptr);
    *suspicious = setup::is_suspicious();
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return -1;
    }

    return key_it->second.m_ts_val.m_timestamp;
}

bool KeyValueStore::put(int64_t key, Value const *val, int64_t timestamp,
                        int64_t *current_timestamp) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        *current_timestamp = timestamp;
        m_store.insert({key, MACedTimestampedValue(
                                 TimestampedValue(val, timestamp, false),
                                 this->get_persistent_pointer(key), key)});
        return true;
    }

    if (key_it->second.m_ts_val.m_timestamp >= timestamp) {
        *current_timestamp = key_it->second.m_ts_val.m_timestamp;
        return false;
    }

    *current_timestamp = timestamp;
    key_it->second =
        MACedTimestampedValue(TimestampedValue(val, timestamp, false),
                              this->get_persistent_pointer(key), key);
    return true;
}

void KeyValueStore::stabilize(int64_t key, int64_t timestamp) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return;
    }
    if (key_it->second.m_ts_val.m_timestamp == timestamp) {
        key_it->second.m_ts_val.m_stable = true;
    }
}

void KeyValueStore::reset() { m_store.clear(); }

}  // namespace restart_rollback
}  // namespace register_sgx
