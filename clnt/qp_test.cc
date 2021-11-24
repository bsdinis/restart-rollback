#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "log.h"
#include "qp_clnt.h"

static int test_n = 1;

#define ASSERT_EQ(val, CONSTANT, description)                     \
    {                                                             \
        do {                                                      \
            fprintf(stderr, "test %d: ", test_n);                 \
            fflush(stderr);                                       \
            if (val == CONSTANT) {                                \
                fprintf(stderr, "success \t| %s\n", description); \
            } else {                                              \
                fprintf(stderr, "fail  \t\t| %s\n", description); \
                fflush(stderr);                                   \
                exit(EXIT_FAILURE);                               \
            }                                                     \
            test_n++;                                             \
        } while (false);                                          \
    }

#define EXPECT_EQ(val, CONSTANT, description)                      \
    {                                                              \
        do {                                                       \
            fprintf(stderr, "test %d: ", test_n);                  \
            fflush(stderr);                                        \
            if (val == CONSTANT) {                                 \
                fprintf(stderr, "success  \t| %s\n", description); \
            } else {                                               \
                fprintf(stderr, "fail  \t\t| %s\n", description);  \
            }                                                      \
            test_n++;                                              \
        } while (false);                                           \
    }

namespace {
void test_sum();
void test_ping();
// XXX: CHANGE ME
// add tests for other operations
void test_pipelining();
void test_cb();

std::string global_config_path = "../QP/default.conf";
ssize_t global_index = 0;
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

using namespace epidemics;  // namespace sanity

int main(int argc, char ** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("starting test");
    ASSERT_EQ(basicQP::init(global_config_path.c_str(), global_index), 0, "init");
    test_sum();
    test_ping();
    test_pipelining();
    test_cb();
    ASSERT_EQ(basicQP::close(), 0, "close");
    INFO("finished test");

    return 0;
}

namespace {
void test_sum() {
    // sync test
    {
        std::vector<int64_t> const v{1, 2, 3, 4};
        EXPECT_EQ(basicQP::sum(v), (int64_t)10, "sum");
    }

    // async test
    {
        std::vector<int64_t> const v{1, 2, 3, 4};
        int64_t const ticket = basicQP::sum_async(v);
        if (basicQP::wait_for(ticket) == basicQP::poll_state::ERR) {
            ERROR("failed to wait for sum");
            return;
        }
        EXPECT_EQ(basicQP::get_reply<int64_t>(ticket), 10, "sum_async");
    }
}

void test_ping() {
    // sync test
    {
        basicQP::ping();
        EXPECT_EQ(0, 0, "ping");
    }

    // async test
    {
        int64_t const ticket = basicQP::ping_async();
        if (basicQP::wait_for(ticket) == basicQP::poll_state::ERR) {
            ERROR("failed to wait for ping");
            return;
        }
        basicQP::get_reply<void>(ticket);
        EXPECT_EQ(0, 0, "ping_async");
    }
}

/**
 * this test tries to verify that both the client and the server
 * perform the pipelining correctly, by sending concurrent requests
 *
 * to see the test in action, place the log level at 2 in both the client and
 * the server
 */
void test_pipelining() {
    std::vector<int64_t> const v{1, 2, 3, 4};
    std::vector<int64_t> sum_tickets;
    std::vector<int64_t> ping_tickets;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0)
            sum_tickets.emplace_back(basicQP::sum_async(v));
        else
            ping_tickets.emplace_back(basicQP::ping_async());
    }

    while (std::any_of(std::cbegin(sum_tickets), std::cend(sum_tickets),
                       [](int64_t ticket) {
                           return basicQP::poll(ticket) ==
                                  basicQP::poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(ping_tickets), std::cend(ping_tickets),
                       [](int64_t ticket) {
                           return basicQP::poll(ticket) ==
                                  basicQP::poll_state::PENDING;
                       }))
        ;

    for (int64_t const ticket : sum_tickets)
        ASSERT_EQ(basicQP::get_reply<int64_t>(ticket), 10, "pipelining");
    for (int64_t const ticket : ping_tickets) {
        basicQP::get_reply<void>(ticket);
        EXPECT_EQ(0, 0, "pipelining");
    }
}

/**
 * this test tries to verify that the callbacks are being properly executed
 */
void test_cb() {
    std::vector<int64_t> const v{1, 2, 3, 4};
    int cbd_funcs = 0;
    ASSERT_EQ(
        basicQP::sum_set_cb([&cbd_funcs](int64_t, int64_t) { cbd_funcs++; }), 0,
        "sum_set_cb");
    ASSERT_EQ(basicQP::ping_set_cb([&cbd_funcs](int64_t) { cbd_funcs++; }), 0,
              "ping_set_cb");

    basicQP::sum_cb(v);
    basicQP::ping_cb();
    basicQP::ping_cb();
    basicQP::sum_cb(v);
    basicQP::wait_for();

    EXPECT_EQ(cbd_funcs, 4, "callback test");
    ASSERT_EQ(basicQP::sum_set_cb([](int64_t, int64_t) {}), 0, "sum_set_cb");
    ASSERT_EQ(basicQP::ping_set_cb([](int64_t) {}), 0, "ping_set_cb");
}

void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n",
            global_config_path.c_str());
    fprintf(stderr, "\t-i [index = %zd]\n", global_index);
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:i:h")) != -1) {
        switch (opt) {
            case 'c':
                global_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'i':
                errno = 0;
                global_index = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
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

}  // anonymous namespace

