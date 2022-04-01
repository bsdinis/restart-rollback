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
#include "qp_clnt.h"

using namespace register_sgx;  // namespace sanity

namespace {
void parse_cli_args(int argc, char** argv);
void print_progress(ssize_t const idx, ssize_t const total);

// cli options
ssize_t global_n_calls = 3000;
std::string global_config_path = "";
std::string global_prefix =
    "";  // all labels in the log file are prefixed with this
std::string global_op = "ping";

using do_func = std::function<void()>;
// XXX: CHANGE ME
// this map generalizes the test for all future ops
//
// when adding a new test, just a new entry to this map
//
// the key is the string which identifies the put
// the second is a lambda to call the put
std::map<std::string, do_func> funcs_by_op = {
    {"ping", []() { crash::ping(); }},
    // if you need to use more intricate things for the arguments, you can add
    // global variables
    {"get",
     []() {
         int64_t timestamp;
         std::array<uint8_t, 2048> value;
         crash::get(2, value, timestamp);
     }},
    {"put",
     []() {
         int64_t timestamp;
         std::array<uint8_t, 2048> value;
         value.fill(1);
         crash::put(3, value, timestamp);
     }},
};

}  // anonymous namespace

int main(int argc, char** argv) {
    setlinebuf(stderr);
    parse_cli_args(argc, argv);
    fprintf(stderr,
            " %s latency test\n"
            " | repetitions: %zd\n"
            " | configuration: %s\n"
            " | prefix: %s\n",
            global_op.c_str(), global_n_calls, global_config_path.c_str(),
            global_prefix.c_str());

    std::time_t const now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    fprintf(stdout,
            "# %s latency test\n"
            "# | repetitions: %zd\n"
            "# | configuration: %s\n"
            "# | prefix: %s\n"
            "# | began at %s"
            "# | everything in us\n",
            global_op.c_str(), global_n_calls, global_config_path.c_str(),
            global_prefix.c_str(), std::ctime(&now));

    if (global_config_path.empty()) {
        if (crash::init() != 0) KILL("failed to init connection to QP");
    } else if (crash::init(global_config_path.c_str()) != 0) {
        KILL("failed to init connection to QP");
    }

    crash::reset();

    for (ssize_t i = 0; i < global_n_calls; i++) {
        auto const start = std::chrono::system_clock::now();
        funcs_by_op.at(global_op)();
        auto const end = std::chrono::system_clock::now();
        auto const elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        fprintf(stdout, "%s%s, %ld\n", global_prefix.c_str(), global_op.c_str(),
                elapsed.count());
        if (i % 10 == 0) print_progress(i, global_n_calls);
    }

    crash::close(true);  // force close
}

namespace {

void usage(char* arg0) {
    std::string known_ops = "|";
    for (auto const& pair : funcs_by_op) {
        known_ops += pair.first + "|";
    }
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n",
            global_config_path.c_str());
    fprintf(stderr, "\t-n [number of calls = %zd]\n", global_n_calls);
    fprintf(stderr, "\t-o [op = %s (%s)]\n", global_op.c_str(),
            known_ops.c_str());
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
                global_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'n':
                errno = 0;
                global_n_calls = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                } else if (global_n_calls <= 0) {
                    fprintf(stderr, "Number of calls needs to be positive: %ld",
                            global_n_calls);
                }

                break;

            case 'o':
                global_op = std::string(optarg, strlen(optarg));
                if (funcs_by_op.find(global_op) == std::end(funcs_by_op)) {
                    std::string known_ops = "|";
                    for (auto const& pair : funcs_by_op) {
                        known_ops += pair.first + "|";
                    }
                    KILL("Put %s is unknown. Know these: %s", global_op.c_str(),
                         known_ops.c_str());
                }
                break;

            case 'p':
                global_prefix = std::string(optarg, strlen(optarg));
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
