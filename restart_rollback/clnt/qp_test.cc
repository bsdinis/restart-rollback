#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "log.h"
#include "qp_clnt.h"

static int test_n = 1;

#define ASSERT_EQ(val, CONSTANT, description)                              \
    {                                                                      \
        do {                                                               \
            fprintf(stderr, "test %4d: ", test_n);                         \
            fflush(stderr);                                                \
            if (val == CONSTANT) {                                         \
                fprintf(stderr, "%10s\t\t| %s\n", "success", description); \
            } else {                                                       \
                fprintf(stderr, "%10s\t\t| %s\n", "fail", description);    \
                fflush(stderr);                                            \
                exit(EXIT_FAILURE);                                        \
            }                                                              \
            test_n++;                                                      \
        } while (false);                                                   \
    }

#define EXPECT_EQ(val, CONSTANT, description)                              \
    {                                                                      \
        do {                                                               \
            fprintf(stderr, "test %4d: ", test_n);                         \
            fflush(stderr);                                                \
            if (val == CONSTANT) {                                         \
                fprintf(stderr, "%10s\t\t| %s\n", "success", description); \
            } else {                                                       \
                fprintf(stderr, "%10s\t\t| %s\n", "fail", description);    \
            }                                                              \
            test_n++;                                                      \
        } while (false);                                                   \
    }

namespace {
void test_fast_get();
void test_get();
void test_transfer();
void test_ping();
void test_pipelining();
void test_cb();

std::string global_config_path = "../QP/default.conf";
ssize_t global_index = 0;
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

using namespace paxos_sgx;  // namespace sanity

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("starting test");
    ASSERT_EQ(restart_rollback::init(global_config_path.c_str(), global_index),
              0, "init");

    restart_rollback::reset();

    test_ping();
    test_fast_get();
    test_get();
    test_transfer();
    test_pipelining();
    test_cb();

    ASSERT_EQ(restart_rollback::close(), 0, "close");
    INFO("finished test");

    return 0;
}

namespace {
void test_fast_get() {
    // sync test
    {
        int64_t amount = 0;
        EXPECT_EQ(restart_rollback::fast_get((int64_t)1, amount), true,
                  "fast_get");
        EXPECT_EQ(amount, (int64_t)1000, "fast_get");
    }

    // async test
    {
        int64_t const ticket = restart_rollback::fast_get_async((int64_t)1);
        if (restart_rollback::wait_for(ticket) ==
            restart_rollback::poll_state::ERR) {
            ERROR("failed to wait for fast_get");
            return;
        }
        auto reply =
            restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket);
        auto expected = std::make_pair<int64_t, bool>((int64_t)1000, true);
        EXPECT_EQ(reply, expected, "fast_get_async");
    }
}

void test_get() {
    // sync test
    {
        int64_t amount = 0;
        EXPECT_EQ(restart_rollback::get(2, amount), true, "get");
        EXPECT_EQ(amount, (int64_t)1000, "get");
    }

    // async test
    {
        int64_t const ticket = restart_rollback::get_async(2);
        if (restart_rollback::wait_for(ticket) ==
            restart_rollback::poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        auto reply =
            restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket);
        auto expected = std::make_pair<int64_t, bool>(1000, true);
        EXPECT_EQ(reply, expected, "get_async");
    }
}

void test_transfer() {
    // sync test
    {
        int64_t amount = 0;
        EXPECT_EQ(restart_rollback::transfer(3, 4, 100, amount), true,
                  "transfer");
        EXPECT_EQ(amount, (int64_t)900, "transfer");
        EXPECT_EQ(restart_rollback::get(3, amount), true, "transfer");
        EXPECT_EQ(amount, (int64_t)900, "transfer");
        EXPECT_EQ(restart_rollback::get(4, amount), true, "transfer");
        EXPECT_EQ(amount, (int64_t)1100, "transfer");
    }

    // async test
    {
        int64_t const ticket = restart_rollback::transfer_async(5, 6, 100);
        if (restart_rollback::wait_for(ticket) ==
            restart_rollback::poll_state::ERR) {
            ERROR("failed to wait for transfer");
            return;
        }
        auto reply =
            restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket);
        auto expected = std::make_pair<int64_t, bool>(900, true);
        EXPECT_EQ(reply, expected, "transfer_async");

        int64_t amount = 0;
        EXPECT_EQ(restart_rollback::get(5, amount), true, "transfer_async");
        EXPECT_EQ(amount, (int64_t)900, "transfer_async");
        EXPECT_EQ(restart_rollback::get(6, amount), true, "transfer_async");
        EXPECT_EQ(amount, (int64_t)1100, "transfer_async");
    }
}

void test_ping() {
    // sync test
    {
        restart_rollback::ping();
        EXPECT_EQ(0, 0, "ping");
    }

    // async test
    {
        int64_t const ticket = restart_rollback::ping_async();
        if (restart_rollback::wait_for(ticket) ==
            restart_rollback::poll_state::ERR) {
            ERROR("failed to wait for ping");
            return;
        }
        restart_rollback::get_reply<void>(ticket);
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
    // std::vector<int64_t> fast_get_tickets;
    std::vector<int64_t> get_tickets;
    std::vector<int64_t> transfer_tickets;
    std::vector<int64_t> ping_tickets;
    for (int i = 0; i < 10; i++) {
        switch (i % 4) {
            case 0:
                // fast_get_tickets.emplace_back(
                // restart_rollback::fast_get_async((int64_t)7));
                // break;
            case 1:
                get_tickets.emplace_back(restart_rollback::get_async(8));
                break;
            case 2:
                transfer_tickets.emplace_back(
                    restart_rollback::transfer_async(9, 10, 1));
                break;
            case 3:
            default:
                ping_tickets.emplace_back(restart_rollback::ping_async());
        }
    }

    while (/*std::any_of(
               std::cbegin(fast_get_tickets), std::cend(fast_get_tickets),
               [](int64_t ticket) {
                   return restart_rollback::poll(ticket) ==
              restart_rollback::poll_state::PENDING;
               }) || */
           std::any_of(std::cbegin(get_tickets), std::cend(get_tickets),
                       [](int64_t ticket) {
                           return restart_rollback::poll(ticket) ==
                                  restart_rollback::poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(transfer_tickets),
                       std::cend(transfer_tickets),
                       [](int64_t ticket) {
                           return restart_rollback::poll(ticket) ==
                                  restart_rollback::poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(ping_tickets), std::cend(ping_tickets),
                       [](int64_t ticket) {
                           return restart_rollback::poll(ticket) ==
                                  restart_rollback::poll_state::PENDING;
                       }))
        ;

    /*
for (int64_t const ticket : fast_get_tickets)
    ASSERT_EQ((restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket)),
              (std::make_pair<int64_t, bool>(1000, true)), "pipelining");
              */
    for (int64_t const ticket : get_tickets)
        ASSERT_EQ(
            (restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket)),
            (std::make_pair<int64_t, bool>(1000, true)), "pipelining");

    int64_t amount = 1000;
    for (int64_t const ticket : transfer_tickets) {
        amount -= 1;
        int64_t rv_amount = amount;
        ASSERT_EQ(
            (restart_rollback::get_reply<std::pair<int64_t, bool>>(ticket)),
            (std::make_pair<int64_t, bool>(std::move(rv_amount), true)),
            "pipelining");
    }
    for (int64_t const ticket : ping_tickets) {
        restart_rollback::get_reply<void>(ticket);
        EXPECT_EQ(0, 0, "pipelining");
    }
}

/**
 * this test tries to verify that the callbacks are being properly executed
 */
void test_cb() {
    int fast_get_n = 0;
    int transfer_n = 0;
    int ping_n = 0;
    ASSERT_EQ(restart_rollback::fast_get_set_cb(
                  [&fast_get_n](int64_t, int64_t, bool) { fast_get_n++; }),
              0, "fast_get_set_cb");
    ASSERT_EQ(restart_rollback::transfer_set_cb(
                  [&transfer_n](int64_t, int64_t, bool) { transfer_n++; }),
              0, "transfer_set_cb");
    ASSERT_EQ(restart_rollback::ping_set_cb([&ping_n](int64_t) { ping_n++; }),
              0, "ping_set_cb");

    restart_rollback::fast_get_cb((int64_t)1);
    restart_rollback::ping_cb();
    restart_rollback::get_cb((int64_t)4);
    restart_rollback::ping_cb();
    restart_rollback::transfer_cb((int64_t)4, (int64_t)6, (int64_t)1);
    restart_rollback::fast_get_cb((int64_t)2);
    restart_rollback::get_cb((int64_t)5);
    restart_rollback::ping_cb();
    restart_rollback::transfer_cb((int64_t)6, (int64_t)4, (int64_t)1);
    restart_rollback::wait_for();

    EXPECT_EQ(fast_get_n, 2, "callback fast get test");
    EXPECT_EQ(transfer_n, 4, "callback transfer test");
    EXPECT_EQ(ping_n, 3, "callback ping test");
    ASSERT_EQ(restart_rollback::fast_get_set_cb([](int64_t, int64_t, bool) {}),
              0, "fast_get_set_cb");
    ASSERT_EQ(restart_rollback::transfer_set_cb([](int64_t, int64_t, bool) {}),
              0, "transfer_set_cb");
    ASSERT_EQ(restart_rollback::ping_set_cb([](int64_t) {}), 0, "ping_set_cb");
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
