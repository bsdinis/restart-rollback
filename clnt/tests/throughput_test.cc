/**
 * throughtput test
 *
 * this test measures the throughput of an put at a certain load
 * it is meant to be changed at will
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "log.h"
#include "teems.h"

using namespace teems;  // namespace sanity

namespace {
// cli options
std::string g_config_path = "../server/default.conf";
size_t g_name_cache_size = 0;
size_t g_value_cache_size = 0;
UntrustedStoreType g_storage_type = UntrustedStoreType::S3;

std::map<std::string, UntrustedStoreType> g_storage_types = {
    {"s3", UntrustedStoreType::S3},
    {"S3", UntrustedStoreType::S3},
    {"fs", UntrustedStoreType::Filesystem},
    {"FS", UntrustedStoreType::Filesystem},
    {"filesystem", UntrustedStoreType::Filesystem},
    {"file", UntrustedStoreType::Filesystem},
    {"redis", UntrustedStoreType::Redis},
    {"Redis", UntrustedStoreType::Redis},
};

// cli options
int64_t g_load = 1000;           // ops/sec
int64_t g_pct_read = 0;          // [0..100]
double g_duration = 5.0;         // sec
double g_warmup_duration = 1.0;  // sec
int64_t g_tick_duration = -1;    // usec

int64_t g_curr_tick = 0;

// number of microseconds since epoch
inline uint64_t now_usecs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

using setup_func = std::function<void()>;
using cb_func = std::function<int64_t()>;
// XXX: CHANGE ME
// this map generalizes the test for all future ops
//
// when adding a new test, just a new entry to this map
//
// the first element is a string which identifies the test
// the second is a lambda to call the put
// the third is a lambda to set the callback after the warmup
std::map<std::string, std::pair<cb_func, setup_func>> funcs_by_op = {
    {"get",
     {// if you need to use more intricate things for the arguments, you can add
      // global variables
      []() -> int64_t { return get_cb(1); },
      []() {
          auto get_callbck = [](int64_t ticket, int64_t, bool,
                                std::vector<uint8_t>, int64_t, int64_t) {
              fprintf(stdout, "%lu, %ld, %ld, %ld, %ld, teems get, reply,\n",
                      now_usecs(), g_load, g_tick_duration, g_curr_tick,
                      ticket);
          };
          if (get_set_cb(get_callbck) == -1) {
              KILL("failed to set fast get callback");
          }
      }}},
    {"put",
     {// if you need to use more intricate things for the arguments, you can add
      // global variables
      []() -> int64_t {
          std::vector<uint8_t> value;
          value.emplace_back(1);
          return put_cb(3, value);
      },
      []() {
          auto put_callbck = [](int64_t ticket, int64_t, bool, int64_t,
                                int64_t) {
              fprintf(stdout, "%lu, %ld, %ld, %ld, %ld, teems put, reply,\n",
                      now_usecs(), g_load, g_tick_duration, g_curr_tick,
                      ticket);
          };
          if (put_set_cb(put_callbck) == -1) {
              KILL("failed to set put callback");
          }
      }}},
};

void parse_cli_args(int argc, char** argv);
void print_progress(ssize_t const idx, ssize_t const total);
// minimum tick duration for there to be one request in each tick
inline int64_t minimum_tick_duration(int64_t load) { return (1000000 / load); }

void warmup(std::chrono::seconds duration) {
    // do 100 reqs per tick at 1000 reqs per second
    std::chrono::microseconds const wtick_duration(minimum_tick_duration(1000) *
                                                   100);

    auto const start = std::chrono::system_clock::now();
    while ((std::chrono::system_clock::now() - start) < duration) {
        auto const tick_start = std::chrono::system_clock::now();
        for (int i = 0; i < 100; i++) {
            if (i % 100 < g_pct_read) {
                (funcs_by_op.at("get").first)();  // call lambda
            } else {
                (funcs_by_op.at("put").first)();  // call lambda
            }
        }

        while ((std::chrono::system_clock::now() - tick_start) <
               wtick_duration) {
            poll();
            print_progress(n_calls_concluded(), n_calls_issued());
        }
    }
    wait_for();
}

void load_test(std::chrono::seconds duration) {
    std::chrono::microseconds const tick_duration(g_tick_duration);

    auto const baseline_calls = n_calls_concluded();
    auto const start = std::chrono::system_clock::now();
    size_t const n_ops = static_cast<size_t>(
        static_cast<double>(g_load * tick_duration.count()) / 1000000.);
    while ((std::chrono::system_clock::now() - start) < duration) {
        auto const tick_start = std::chrono::system_clock::now();
        for (size_t i = 0; i < n_ops; i++) {
            if (i % 100 < g_pct_read) {
                int64_t const ticket =
                    (funcs_by_op.at("get").first)();  // call lambda
                fprintf(
                    stdout, "%lu, %ld, %ld, %ld, %ld, teems get, request,\n",
                    now_usecs(), g_load, g_tick_duration, g_curr_tick, ticket);
            } else {
                int64_t const ticket =
                    (funcs_by_op.at("put").first)();  // call lambda
                fprintf(
                    stdout, "%lu, %ld, %ld, %ld, %ld, teems put, request,\n",
                    now_usecs(), g_load, g_tick_duration, g_curr_tick, ticket);
            }
        }

        while ((std::chrono::system_clock::now() - tick_start) <
               tick_duration) {
            print_progress(n_calls_concluded() - baseline_calls,
                           n_calls_issued() - baseline_calls);
            poll();
        }
        g_curr_tick += 1;
    }
}
}  // anonymous namespace

int main(int argc, char** argv) {
    setlinebuf(stderr);
    parse_cli_args(argc, argv);
    fprintf(stderr,
            " | %ld %% test\n"
            " | load: %zd\n"
            " | configuration: %s\n"
            " | duration: %lf\n"
            " | tick duration: %ld\n"
            " | name cache size: %zu\n"
            " | value cache size: %zu\n",
            g_pct_read, g_load, g_config_path.c_str(), g_duration,
            g_tick_duration, g_name_cache_size, g_value_cache_size);

    std::time_t const now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    fprintf(stdout,
            "# | %ld %% test\n"
            "# | load: %zd\n"
            "# | configuration: %s\n"
            "# | duration: %lf\n"
            "# | tick duration: %ld\n"
            "# | name cache size: %zu\n"
            "# | value cache size: %zu\n"
            "# | began at %s"
            "# | everything in us\n",
            g_pct_read, g_load, g_config_path.c_str(), g_duration,
            g_tick_duration, g_name_cache_size, g_value_cache_size,
            std::ctime(&now));
    fflush(stdout);

    if (init(g_config_path.c_str(), g_storage_type, g_name_cache_size,
             g_value_cache_size) != 0) {
        KILL("failed to init connection to server");
    }

    reset();

    INFO("starting warmup");
    // warmup
    std::chrono::seconds warmup_duration(
        static_cast<int>(g_warmup_duration + 1));
    warmup(warmup_duration);
    INFO("finished warmup");

    // change the callbacks
    (funcs_by_op.at("get").second)();
    (funcs_by_op.at("put").second)();

    // test
    std::chrono::seconds test_duration(static_cast<int>(g_duration + 1));
    load_test(test_duration);
    INFO("finished test");
    close(true);
}

namespace {

void usage(char* arg0) {
    std::string known_ops = "|";
    for (auto const& pair : funcs_by_op) {
        known_ops += pair.first + "|";
    }
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n", g_config_path.c_str());
    fprintf(stderr, "\t-d [duration = %3.4f sec]\n", g_duration);
    fprintf(stderr, "\t-l [load = %ld ops/sec]\n", g_load);
    fprintf(stderr, "\t-o [op = %ld (0..100)]\n", g_pct_read);
    fprintf(stderr, "\t-t [tick duration = %ld usec]\n", g_tick_duration);
    fprintf(stderr, "\t-w [warmup duration = %3.4f sec]\n", g_warmup_duration);
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:d:l:o:t:h")) != -1) {
        switch (opt) {
            case 'a':
                errno = 0;
                g_name_cache_size = std::stoull(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }

                break;

            case 'b':
                errno = 0;
                g_value_cache_size = std::stoull(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }

                break;

            case 'c':
                g_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'd':
                errno = 0;
                g_duration = std::stod(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as double: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (g_duration <= 0.0)
                    KILL("Duration needs to be positive: %ld", g_tick_duration);
                break;

            case 'l':
                errno = 0;
                g_load = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (g_load <= 0)
                    KILL("Load needs to be positive: %ld", g_tick_duration);

                break;

            case 'o':
                g_pct_read = std::stoll(optarg);
                if (g_pct_read < 0 || g_pct_read > 100) {
                    KILL("invalid pct: %ld g_pct_read", g_pct_read);
                }
                break;

            case 's': {
                auto storage_type = std::string(optarg, strlen(optarg));
                if (g_storage_types.find(storage_type) ==
                    std::end(g_storage_types)) {
                    std::string known_types = "|";
                    for (auto const& pair : g_storage_types) {
                        known_types += pair.first + "|";
                    }
                    KILL("Store type %s is unknown. Know these: %s",
                         storage_type.c_str(), known_types.c_str());
                }

                g_storage_type = g_storage_types[storage_type];
            } break;

            case 't':
                errno = 0;
                g_tick_duration = std::stol(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (g_tick_duration <= 0)
                    KILL("Tick Duration needs to be positive: %ld",
                         g_tick_duration);
                break;

            case 'w':
                errno = 0;
                g_warmup_duration = std::stod(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as double: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (g_warmup_duration <= 0.0)
                    KILL("Warmup Duration needs to be positive: %ld",
                         g_tick_duration);
                break;

            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;

            default: /* '?' */
                ERROR("Unknown flag -%c", optopt);
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (g_tick_duration == -1) {
        g_tick_duration = std::max(50000L, minimum_tick_duration(g_load) * 50);
    } else {
        g_tick_duration = std::max(50000L, g_tick_duration);
    }
}

void print_progress(ssize_t idx, ssize_t total) {
    if (total <= 0) KILL("cannot have non positive total");

    idx++;
    int bar_size = 30;
    int progress = idx * bar_size / total;

    fprintf(stderr, "%4.0f%% [", ((double)idx * 100) / total);
    int i = 0;
    while (i < progress) {
        fprintf(stderr, "=");
        i++;
    }

    while (i < bar_size) {
        fprintf(stderr, " ");
        i++;
    }

    fprintf(stderr, "] %zu/%zd", idx, total);
    fprintf(stderr, "\r");
    fflush(stderr);
}

}  // anonymous namespace
