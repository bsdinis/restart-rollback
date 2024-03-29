#include "kv_store.h"
#include "crypto_helpers.h"
#include "log.h"

namespace register_sgx {
namespace byzantine {

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

int64_t KeyValueStore::get(int64_t key, std::array<uint8_t, REGISTER_SIZE> *val,
                           std::vector<uint8_t> &signature) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return -1;
    }

    int64_t timestamp = key_it->second.m_ts_val.m_timestamp;
    *val = key_it->second.m_ts_val.m_val;
    signature.clear();
    std::copy(std::begin(key_it->second.m_ts_val.m_signature),
              std::end(key_it->second.m_ts_val.m_signature),
              std::back_inserter(signature));
    return timestamp;
}

int64_t KeyValueStore::get_timestamp(int64_t key) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return -1;
    }

    return key_it->second.m_ts_val.m_timestamp;
}

bool KeyValueStore::put(int64_t key, DataValue const *val, int64_t timestamp,
                        std::vector<uint8_t> const &signature,
                        int64_t *current_timestamp) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        *current_timestamp = timestamp;
        m_store.insert({key, MACedTimestampedValue(
                                 TimestampedValue(val, timestamp, signature),
                                 this->get_persistent_pointer(key), key)});
        return true;
    }

    if (key_it->second.m_ts_val.m_timestamp >= timestamp) {
        *current_timestamp = key_it->second.m_ts_val.m_timestamp;
        return false;
    }

    *current_timestamp = timestamp;
    key_it->second =
        MACedTimestampedValue(TimestampedValue(val, timestamp, signature),
                              this->get_persistent_pointer(key), key);
    return true;
}

void KeyValueStore::reset() { m_store.clear(); }

}  // namespace byzantine
}  // namespace register_sgx
