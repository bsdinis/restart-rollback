#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

#include "log.h"
#include "lru.h"

static int test_n = 1;

#define ASSERT(condition, description)                                     \
    {                                                                      \
        do {                                                               \
            fprintf(stderr, "test %4d: ", test_n);                         \
            fflush(stderr);                                                \
            if (condition) {                                               \
                fprintf(stderr, "%10s\t\t| %s\n", "success", description); \
            } else {                                                       \
                fprintf(stderr, "%10s\t\t| %s\n", "fail", description);    \
                fflush(stderr);                                            \
                exit(EXIT_FAILURE);                                        \
            }                                                              \
            test_n++;                                                      \
        } while (false);                                                   \
    }

#define EXPECT(condition, description)                                     \
    {                                                                      \
        do {                                                               \
            fprintf(stderr, "test %4d: ", test_n);                         \
            fflush(stderr);                                                \
            if (condition) {                                               \
                fprintf(stderr, "%10s\t\t| %s\n", "success", description); \
            } else {                                                       \
                fprintf(stderr, "%10s\t\t| %s\n", "fail", description);    \
            }                                                              \
            test_n++;                                                      \
        } while (false);                                                   \
    }

template <>
size_t teems::get_size_for_lru<int>(int const &v) {
    return sizeof(int);
}

template <>
size_t teems::get_size_for_lru<int64_t>(int64_t const &v) {
    return sizeof(int64_t);
}

using namespace teems;  // namespace sanity
                        //
namespace {
void test_basic_int();
void test_vector();
void test_too_big();
}  // namespace

int main() {
    test_basic_int();
    test_vector();
    test_too_big();

    return 0;
}

namespace {
void test_basic_int() {
    LRUCache<int, int> cache(sizeof(int) * 2);

    int const *value = nullptr;

    cache.insert(1, 1);
    value = cache.get(1);
    ASSERT(value != nullptr, "value 1");
    EXPECT(*value == 1, "value 1");

    cache.insert(2, 2);
    value = cache.get(2);
    ASSERT(value != nullptr, "value 2");
    EXPECT(*value == 2, "value 2");

    cache.insert(3, 3);
    value = cache.get(3);
    ASSERT(value != nullptr, "value 3");
    EXPECT(*value == 3, "value 3");

    value = cache.get(1);
    EXPECT(value == nullptr, "value 1 was evicted");

    value = cache.get(2);
    ASSERT(value != nullptr, "value 2 was not evicted");
    EXPECT(*value == 2, "value 2");

    cache.insert(4, 4);
    value = cache.get(4);
    ASSERT(value != nullptr, "value 4");
    EXPECT(*value == 4, "value 4");

    value = cache.get(3);
    EXPECT(value == nullptr, "value 3 was evicted");
}

void test_vector() {
    LRUCache<int, std::vector<uint8_t>> cache(10 * sizeof(uint8_t));

    std::vector<uint8_t> vec;

    cache.insert(0, vec);
    EXPECT(cache.get(0) != nullptr, "[]");

    vec.emplace_back(1);
    cache.insert(1, vec);
    EXPECT(cache.get(0) != nullptr, "[], [1]");
    EXPECT(cache.get(1) != nullptr, "[], [1]");

    vec.emplace_back(2);
    cache.insert(2, vec);
    EXPECT(cache.get(0) != nullptr, "[], [1], [1, 2]");
    EXPECT(cache.get(1) != nullptr, "[], [1], [1, 2]");
    EXPECT(cache.get(2) != nullptr, "[], [1], [1, 2]");

    vec.emplace_back(3);
    cache.insert(3, vec);
    EXPECT(cache.get(0) != nullptr, "[], [1], [1, 2], [1, 2, 3]");
    EXPECT(cache.get(1) != nullptr, "[], [1], [1, 2], [1, 2, 3]");
    EXPECT(cache.get(2) != nullptr, "[], [1], [1, 2], [1, 2, 3]");
    EXPECT(cache.get(3) != nullptr, "[], [1], [1, 2], [1, 2, 3]");

    vec.emplace_back(4);
    cache.insert(4, vec);
    EXPECT(cache.get(0) != nullptr, "[], [1], [1, 2], [1, 2, 3], [1, 2, 3, 4]");
    EXPECT(cache.get(1) != nullptr, "[], [1], [1, 2], [1, 2, 3], [1, 2, 3, 4]");
    EXPECT(cache.get(2) != nullptr, "[], [1], [1, 2], [1, 2, 3], [1, 2, 3, 4]");
    EXPECT(cache.get(3) != nullptr, "[], [1], [1, 2], [1, 2, 3], [1, 2, 3, 4]");
    EXPECT(cache.get(4) != nullptr, "[], [1], [1, 2], [1, 2, 3], [1, 2, 3, 4]");

    vec.emplace_back(5);
    cache.insert(5, vec);
    EXPECT(cache.get(0) == nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
    EXPECT(cache.get(1) == nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
    EXPECT(cache.get(2) == nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
    EXPECT(cache.get(3) == nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
    EXPECT(cache.get(4) != nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
    EXPECT(cache.get(5) != nullptr, "[1, 2, 3, 4], [1, 2, 3, 4, 5]");
}

void test_too_big() {
    LRUCache<int, int64_t> cache(sizeof(int8_t));

    cache.insert(0, 0);
    EXPECT(cache.get(0) == nullptr, "value is too big for cache");
}
}  // namespace
