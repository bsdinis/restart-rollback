#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace teems {

void add_hint(int64_t key, std::string const &name,
              std::vector<uint8_t> const &value);

void reset_name_cache(size_t capacity);
size_t name_hits();
size_t name_misses();

void reset_value_cache(size_t capacity);
size_t value_hits();
size_t value_misses();

}  // namespace teems
