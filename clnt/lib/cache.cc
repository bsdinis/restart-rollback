#include "cache.h"
#include "lru.h"

namespace teems {

// we count the size of the name cache in number of names
template <>
size_t get_size_for_lru<std::string>(std::string const &a) {
    return 1;
}

// we count the size of the value cache in number of bytes
template <>
size_t get_size_for_lru<std::pair<std::string, std::vector<uint8_t>>>(
    std::pair<std::string, std::vector<uint8_t>> const &v) {
    return v.second.size() * sizeof(uint8_t);
}

// stores name hints for TEEMS
LRUCache<int64_t, std::string> g_name_cache(0);

// stores value hints for TEEMS
LRUCache<int64_t, std::pair<std::string, std::vector<uint8_t>>> g_value_cache(
    0);

void add_hint(int64_t key, std::string const &name,
              std::vector<uint8_t> const &value) {
    g_name_cache.insert(key, name);
    g_value_cache.insert(key, std::make_pair(name, value));
}

void reset_name_cache(size_t capacity) {
    g_name_cache = LRUCache<int64_t, std::string>(capacity);
}
size_t name_hits() { return g_name_cache.hits(); }
size_t name_misses() { return g_name_cache.misses(); }

void reset_value_cache(size_t capacity) {
    g_value_cache =
        LRUCache<int64_t, std::pair<std::string, std::vector<uint8_t>>>(
            capacity);
}
size_t value_hits() { return g_value_cache.hits(); }
size_t value_misses() { return g_value_cache.misses(); }

}  // namespace teems
