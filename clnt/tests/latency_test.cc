/**
 * latency test
 *
 * this test measures the latency of an put
 *
 * it is meant to be changed at will
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "log.h"
#include "teems.h"

using namespace teems;  // namespace sanity

namespace {
void parse_cli_args(int argc, char** argv);
void print_progress(ssize_t const idx, ssize_t const total);

// cli options
ssize_t g_n_calls = 3000;
size_t g_name_cache_size = 0;
size_t g_value_cache_size = 0;
UntrustedStoreType g_storage_type = UntrustedStoreType::S3;
std::string g_config_path = "../server/default.conf";
std::string g_prefix = "";  // all labels in the log file are prefixed with this
std::string g_op = "ping";

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

using do_func = std::function<void()>;

// XXX: CHANGE ME
// this map generalizes the test for all future ops
//
// when adding a new test, just a new entry to this map
//
// the key is the string which identifies the put
// the second is a lambda to call the put
std::map<std::string, do_func> funcs_by_op = {
    {"ping", []() { ping(); }},
    // if you need to use more intricate things for the arguments, you can add
    // global variables
    {"get",
     []() {
         int64_t policy_version;
         int64_t timestamp;
         std::vector<uint8_t> value;
         get(2, value, policy_version, timestamp);
     }},
    {"put",
     []() {
         int64_t policy_version;
         int64_t timestamp;
         std::vector<uint8_t> value;
         value.emplace_back(1);
         put(3, value, policy_version, timestamp);
     }},
};

}  // anonymous namespace

int main(int argc, char** argv) {
    setlinebuf(stderr);
    parse_cli_args(argc, argv);
    fprintf(stderr,
            " %s latency test\n"
            " | repetitions: %zd\n"
            " | name cache size: %zu\n"
            " | value cache size: %zu\n"
            " | configuration: %s\n"
            " | prefix: %s\n",
            g_op.c_str(), g_n_calls, g_name_cache_size, g_value_cache_size,
            g_config_path.c_str(), g_prefix.c_str());

    std::time_t const now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    fprintf(stdout,
            "# %s latency test\n"
            "# | repetitions: %zd\n"
            "# | name cache size: %zu\n"
            "# | value cache size: %zu\n"
            "# | configuration: %s\n"
            "# | prefix: %s\n"
            "# | began at %s"
            "# | everything in us\n",
            g_op.c_str(), g_n_calls, g_name_cache_size, g_value_cache_size,
            g_config_path.c_str(), g_prefix.c_str(), std::ctime(&now));

    if (init(g_config_path.c_str(), g_storage_type, g_name_cache_size,
             g_value_cache_size) != 0) {
        KILL("failed to init connection to server");
    }

    reset();

    for (ssize_t i = 0; i < g_n_calls; i++) {
        auto const start = std::chrono::system_clock::now();
        funcs_by_op.at(g_op)();
        auto const end = std::chrono::system_clock::now();
        auto const elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        fprintf(stdout, "%s%s, %ld\n", g_prefix.c_str(), g_op.c_str(),
                elapsed.count());
        if (i % 10 == 0) print_progress(i, g_n_calls);
    }

    close(true);  // force close
}

namespace {

void usage(char* arg0) {
    std::string known_ops = "|";
    for (auto const& pair : funcs_by_op) {
        known_ops += pair.first + "|";
    }
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n", g_config_path.c_str());
    fprintf(stderr, "\t-n [number of calls = %zd]\n", g_n_calls);
    fprintf(stderr, "\t-o [op = %s (%s)]\n", g_op.c_str(), known_ops.c_str());
    fprintf(stderr, "\t-p [prefix for log]\n");
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:n:o:p:h")) != -1) {
        switch (opt) {
            case 'c':
                g_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'n':
                errno = 0;
                g_n_calls = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                } else if (g_n_calls <= 0) {
                    fprintf(stderr, "Number of calls needs to be positive: %ld",
                            g_n_calls);
                }

                break;

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

            case 'o':
                g_op = std::string(optarg, strlen(optarg));
                if (funcs_by_op.find(g_op) == std::end(funcs_by_op)) {
                    std::string known_ops = "|";
                    for (auto const& pair : funcs_by_op) {
                        known_ops += pair.first + "|";
                    }
                    KILL("Put %s is unknown. Know these: %s", g_op.c_str(),
                         known_ops.c_str());
                }
                break;

            case 'p':
                g_prefix = std::string(optarg, strlen(optarg));
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
