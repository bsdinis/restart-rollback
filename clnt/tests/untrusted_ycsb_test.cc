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
#include "ycsb_trace.h"

using namespace teems;  // namespace sanity

namespace {
void parse_cli_args(int argc, char** argv);
void print_progress(ssize_t const idx, ssize_t const total);

// cli options
size_t g_name_cache_size = 0;
size_t g_value_cache_size = 0;
UntrustedStoreType g_storage_type = UntrustedStoreType::S3;
std::string g_trace_path = "";
std::string g_config_path = "../server/default.conf";
std::string g_prefix = "";  // all labels in the log file are prefixed with this

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

}  // anonymous namespace

int main(int argc, char** argv) {
    setlinebuf(stderr);
    parse_cli_args(argc, argv);

    fprintf(stderr,
            " ycsb test\n"
            " | trace file: %s\n"
            " | name cache size: %zu\n"
            " | value cache size: %zu\n"
            " | configuration: %s\n"
            " | prefix: %s\n",
            g_trace_path.c_str(), g_name_cache_size, g_value_cache_size,
            g_config_path.c_str(), g_prefix.c_str());

    auto trace = ycsb::import_trace(g_trace_path);

    std::time_t const now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    fprintf(stdout,
            "# latency test\n"
            "# | trace file: %s\n"
            "# | name cache size: %zu\n"
            "# | value cache size: %zu\n"
            "# | configuration: %s\n"
            "# | prefix: %s\n"
            "# | began at %s"
            "# | everything in us\n",
            g_trace_path.c_str(), g_name_cache_size, g_value_cache_size,
            g_config_path.c_str(), g_prefix.c_str(), std::ctime(&now));

    if (init(g_config_path.c_str(), g_storage_type, g_name_cache_size,
             g_value_cache_size) != 0) {
        KILL("failed to init connection to server");
    }

    reset();

    size_t n_call = 0;
    for (auto op : trace) {
        op.do_untrusted_operation();
        if (n_call % 10 == 0) {
            print_progress(n_call, trace.size());
        }

        n_call += 1;
    }

    close(true);  // force close
}

namespace {

void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n", g_config_path.c_str());
    fprintf(stderr, "\t-t [trace file = %s]\n", g_trace_path.c_str());
    fprintf(stderr, "\t-p [prefix for log]\n");
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "a:b:c:t:p:s:h")) != -1) {
        switch (opt) {
            case 'c':
                g_config_path = std::string(optarg, strlen(optarg));
                break;

            case 't':
                g_trace_path = std::string(optarg, strlen(optarg));
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
