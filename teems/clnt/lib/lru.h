#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>

namespace teems {

// tuning knob:
// can override this for any type to change how the size is measured by the LRU
// cache
template <typename V>
size_t get_size_for_lru(V const &v) {
    return sizeof(V);
}

template <typename K, typename V>
class LRUCache {
   public:
    LRUCache(size_t max_capacity) : m_max_capacity(max_capacity) {}

    V const *get(K const &key);
    void insert(K key, V value);

    inline size_t hits() { return m_hits; }
    inline size_t misses() { return m_misses; }

   private:
    // list of (key, value size) ordered by time of access
    std::list<std::pair<K, size_t>> m_lru_keys;

    std::unordered_map<
        K, std::pair<typename std::list<std::pair<K, size_t>>::iterator, V>>
        m_map;
    size_t m_max_capacity = 0;
    size_t m_size = 0;

    size_t m_hits = 0;
    size_t m_misses = 0;
};

template <typename K, typename V>
void LRUCache<K, V>::insert(K key, V value) {
    if (get_size_for_lru(value) > m_max_capacity) {
        // value is too big for the cache
        return;
    }

    if (m_map.find(key) == m_map.end()) {
        // full cache
        while (m_size + get_size_for_lru(value) > m_max_capacity) {
            // delete least recently used element
            K last_key;
            size_t size = 0;
            std::tie(last_key, size) = m_lru_keys.back();

            m_lru_keys.pop_back();

            // Erase the last
            m_map.erase(last_key);
            m_size -= size;
        }
    } else {
        // deduct size from the old value under this key
        m_size -= m_map[key].first->second;

        // remove current reference in the LRU list
        m_lru_keys.erase(m_map[key].first);
    }

    m_lru_keys.push_front(std::make_pair(key, get_size_for_lru(value)));
    m_map[key] = std::make_pair(m_lru_keys.begin(), value);
    m_size += get_size_for_lru(value);
}

template <typename K, typename V>
V const *LRUCache<K, V>::get(K const &key) {
    if (m_map.find(key) == m_map.end()) {
        m_misses += 1;
        return nullptr;
    }

    // update LRU list
    size_t value_size = m_map[key].first->second;
    m_lru_keys.erase(m_map[key].first);
    m_lru_keys.push_front(std::make_pair(key, value_size));
    m_map[key].first = m_lru_keys.begin();

    m_hits += 1;
    return &m_map[key].second;
}

}  // namespace teems
