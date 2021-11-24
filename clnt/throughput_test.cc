/**
 * throughtput test
 *
 * this test measures the throughput of an operation at a certain load
 * it is meant to be changed at will
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "log.h"
#include "qp_clnt.h"

using namespace epidemics;  // namespace sanity

namespace {
// cli options
std::string global_config_path = "";

// cli options
int64_t global_load = 1000;           // ops/sec
double global_duration = 5.0;         // sec
double global_warmup_duration = 1.0;  // sec
int64_t global_tick_duration = -1;    // usec
std::string global_op = "ping";

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
// the second is a lambda to call the operation
// the third is a lambda to set the callback after the warmup
std::map<std::string, std::pair<cb_func, setup_func>> funcs_by_op = {
    {"ping",
     {[]() -> int64_t { return basicQP::ping_cb(); },
      []() {
          auto ping_callbck = [](int64_t ticket) {
              fprintf(stdout, "%lu, %ld, %ld, %ld, PING\n", now_usecs(),
                      global_load, global_tick_duration, ticket);
          };
          if (basicQP::ping_set_cb(ping_callbck) == -1) {
              KILL("failed to set ping callback");
          }
      }}},
    {"sum",
     {// if you need to use more intricate things for the arguments, you can add
      // global variables
      []() -> int64_t {
          return basicQP::sum_cb(std::vector<int64_t>{1, 2, 3, 4});
      },
      []() {
          auto sum_callbck = [](int64_t ticket, int64_t) {
              fprintf(stdout, "%lu, %ld, %ld, %ld, SUM\n", now_usecs(),
                      global_load, global_tick_duration, ticket);
          };
          if (basicQP::sum_set_cb(sum_callbck) == -1) {
              KILL("failed to set sum callback");
          }
      }}},
};

void parse_cli_args(int argc, char** argv);
void print_progress(ssize_t const idx, ssize_t const total);
// minimum tick duration for there to be one request in each tick
inline int64_t minimum_tick_duration(int64_t load) { return (1000000 / load); }

// XXX: CHANGE ME
void warmup(std::chrono::seconds duration) {
    // do 100 reqs per tick at 1000 reqs per second
    std::chrono::microseconds const wtick_duration(minimum_tick_duration(1000) *
                                                   100);

    auto const start = std::chrono::system_clock::now();
    while ((std::chrono::system_clock::now() - start) < duration) {
        auto const tick_start = std::chrono::system_clock::now();
        for (int i = 0; i < 100; i++)
            (funcs_by_op.at(global_op).first)();  // call lambda

        while ((std::chrono::system_clock::now() - tick_start) <
               wtick_duration) {
            basicQP::poll();
            print_progress(basicQP::n_calls_concluded(),
                           basicQP::n_calls_issued());
        }
    }
    basicQP::wait_for();
}

// XXX: CHANGE ME
void load_test(std::chrono::seconds duration) {
    std::chrono::microseconds const tick_duration(global_tick_duration);

    auto const baseline_calls = basicQP::n_calls_concluded();
    auto const start = std::chrono::system_clock::now();
    size_t const n_ops = static_cast<size_t>(
        static_cast<double>(global_load * tick_duration.count()) / 1000000.);
    while ((std::chrono::system_clock::now() - start) < duration) {
        auto const tick_start = std::chrono::system_clock::now();
        for (size_t i = 0; i < n_ops; i++) {
            int64_t const ticket =
                (funcs_by_op.at(global_op).first)();  // call lambda
            fprintf(stdout, "%lu, %ld, %ld, %ld, %s\n", now_usecs(),
                    global_load, global_tick_duration, ticket,
                    global_op.c_str());
        }

        while ((std::chrono::system_clock::now() - tick_start) <
               tick_duration) {
            print_progress(basicQP::n_calls_concluded() - baseline_calls,
                           basicQP::n_calls_issued() - baseline_calls);
            basicQP::poll();
        }
    }
    basicQP::wait_for();
}
}  // anonymous namespace

int main(int argc, char** argv) {
    setlinebuf(stderr);
    parse_cli_args(argc, argv);
    fprintf(stderr,
            " %s throughput test\n"
            " | load: %zd\n"
            " | configuration: %s\n"
            " | duration: %lf\n"
            " | tick duration: %ld\n",
            global_op.c_str(), global_load, global_config_path.c_str(),
            global_duration, global_tick_duration);

    std::time_t const now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    fprintf(stdout,
            "# %s throughput test\n"
            "# | load: %zd\n"
            "# | configuration: %s\n"
            "# | duration: %lf\n"
            "# | tick duration: %ld\n"
            "# | began at %s"
            "# | everything in us\n",
            global_op.c_str(), global_load, global_config_path.c_str(),
            global_duration, global_tick_duration, std::ctime(&now));
    fflush(stdout);

    if (global_config_path.empty()) {
        if (basicQP::init() != 0)
            KILL("failed to init connection to QP");
    } else if (basicQP::init(global_config_path.c_str()) != 0) {
        KILL("failed to init connection to QP");
    }

    INFO("starting warmup");
    // warmup
    std::chrono::seconds warmup_duration(
        static_cast<int>(global_warmup_duration + 1));
    warmup(warmup_duration);
    INFO("finished warmup");

    // change the callback
    (funcs_by_op.at(global_op).second)();  // call lambda

    // test
    std::chrono::seconds test_duration(static_cast<int>(global_duration + 1));
    load_test(test_duration);
    INFO("finished test");
    basicQP::close(true); // force close
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
    fprintf(stderr, "\t-d [duration = %3.4f sec]\n", global_duration);
    fprintf(stderr, "\t-l [load = %ld ops/sec]\n", global_load);
    fprintf(stderr, "\t-o [op = %s (%s)]\n", global_op.c_str(),
            known_ops.c_str());
    fprintf(stderr, "\t-t [tick duration = %ld usec]\n", global_tick_duration);
    fprintf(stderr, "\t-w [warmup duration = %3.4f sec]\n",
            global_warmup_duration);
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:d:l:o:t:h")) != -1) {
        switch (opt) {
            case 'c':
                global_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'd':
                errno = 0;
                global_duration = std::stod(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as double: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (global_duration <= 0.0)
                    KILL("Duration needs to be positive: %ld",
                         global_tick_duration);
                break;

            case 'l':
                errno = 0;
                global_load = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (global_load <= 0)
                    KILL("Load needs to be positive: %ld",
                         global_tick_duration);

                break;

            case 'o':
                global_op = std::string(optarg, strlen(optarg));
                if (funcs_by_op.find(global_op) == std::end(funcs_by_op)) {
                    std::string known_ops = "|";
                    for (auto const& pair : funcs_by_op) {
                        known_ops += pair.first + "|";
                    }
                    KILL("Operation %s is unknown. Know these: %s",
                         global_op.c_str(), known_ops.c_str());
                }
                break;

            case 't':
                errno = 0;
                global_tick_duration = std::stol(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (global_tick_duration <= 0)
                    KILL("Tick Duration needs to be positive: %ld",
                         global_tick_duration);
                break;

            case 'w':
                errno = 0;
                global_warmup_duration = std::stod(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as double: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (global_warmup_duration <= 0.0)
                    KILL("Warmup Duration needs to be positive: %ld",
                         global_tick_duration);
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

    if (global_tick_duration == -1) {
        global_tick_duration =
            std::max(50000L, minimum_tick_duration(global_load) * 50);
    } else {
        global_tick_duration = std::max(50000L, global_tick_duration);
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
