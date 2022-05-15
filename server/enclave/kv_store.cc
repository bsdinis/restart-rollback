#include "kv_store.h"

#include "crypto_helpers.h"
#include "log.h"
#include "setup.h"

namespace teems {

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

bool KeyValueStore::get(int64_t key, bool *stable, bool *suspicious,
                        int64_t *policy_version, int64_t *timestamp,
                        ServerPolicy *policy,
                        std::array<uint8_t, REGISTER_SIZE> *val) {
    assert(stable != nullptr);
    assert(suspicious != nullptr);
    *suspicious = setup::is_suspicious();

    *policy = ServerPolicy();
    *policy_version = -1;
    *timestamp = -1;

    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        *stable = true;
        return false;
    }

    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        ERROR("there is a value but there is no policy");
        *stable = true;
        return false;
    }

    std::tie(*policy_version, *policy) = policy_it->second;
    *timestamp = key_it->second.m_ts_val.m_timestamp;
    *stable = key_it->second.m_ts_val.m_stable;
    *val = key_it->second.m_ts_val.m_val;

    return true;
}

bool KeyValueStore::get_timestamp(int64_t key, bool *stable, bool *suspicious,
                                  int64_t *policy_version, int64_t *timestamp,
                                  ServerPolicy *policy) {
    assert(suspicious != nullptr);
    *suspicious = setup::is_suspicious();

    *policy = ServerPolicy();
    *policy_version = -1;
    *timestamp = -1;

    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return false;
    }

    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        ERROR("there is a value but there is no policy");
        *stable = true;
        return false;
    }

    std::tie(*policy_version, *policy) = policy_it->second;
    *timestamp = key_it->second.m_ts_val.m_timestamp;
    return true;
}

bool KeyValueStore::put(int64_t key, Value const *val,
                        ServerPolicy const &policy, int64_t policy_version,
                        int64_t timestamp, int64_t *current_policy_version,
                        int64_t *current_timestamp) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        *current_policy_version = std::max<int64_t>(policy_version, 0);
        *current_timestamp = timestamp;
        m_store.insert({key, MACedTimestampedValue(
                                 TimestampedValue(val, timestamp, false),
                                 this->get_persistent_pointer(key), key)});
        m_policy_store.insert(
            {key, std::make_tuple(*current_policy_version, policy)});
        return true;
    }

    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        ERROR("there is a value but there is no policy");
        return false;
    }

    if (key_it->second.m_ts_val.m_timestamp >= timestamp) {
        *current_policy_version = std::get<0>(policy_it->second);
        *current_timestamp = key_it->second.m_ts_val.m_timestamp;
        return false;
    }

    *current_policy_version = policy_version;
    *current_timestamp = timestamp;
    key_it->second =
        MACedTimestampedValue(TimestampedValue(val, timestamp, false),
                              this->get_persistent_pointer(key), key);
    return true;
}

void KeyValueStore::stabilize(int64_t key, int64_t policy_version,
                              int64_t timestamp) {
    auto key_it = m_store.find(key);
    if (key_it == m_store.end()) {
        return;
    }
    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        ERROR("found register but didn't find the policy");
        return;
    }
    if (std::get<0>(policy_it->second) == policy_version &&
        key_it->second.m_ts_val.m_timestamp == timestamp) {
        key_it->second.m_ts_val.m_stable = true;
    }
}

bool KeyValueStore::get_policy(int64_t key, ServerPolicy *policy,
                               int64_t *policy_version) {
    *policy_version = -1;

    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        *policy = ServerPolicy();
        return false;
    }

    std::tie(*policy_version, *policy) = policy_it->second;
    return true;
}

bool KeyValueStore::change_policy(int64_t key, ServerPolicy policy,
                                  int64_t *policy_version) {
    auto policy_it = m_policy_store.find(key);
    if (policy_it == m_policy_store.end()) {
        *policy_version = -1;
        return false;
    }

    if (!std::get<1>(policy_it->second).can_change_policy(policy.owner_id())) {
        *policy_version = std::get<0>(policy_it->second);
        return false;
    }

    *policy_version = std::get<0>(policy_it->second) + 1;
    policy_it->second = std::make_tuple(*policy_version, policy);
    return true;
}

void KeyValueStore::reset() { m_store.clear(); }

}  // namespace teems
