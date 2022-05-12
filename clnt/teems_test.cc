#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "log.h"
#include "teems.h"

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
void test_get_put();
void test_ping();
void test_pipelining();
void test_cb();

std::string global_config_path = "../server/default.conf";
ssize_t global_index = 0;
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

using namespace teems;  // namespace sanity

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("starting test");
    ASSERT_EQ(init(global_config_path.c_str(), global_index), 0, "init");

    reset();

    test_ping();
    test_get_put();
    test_pipelining();
    test_cb();

    ASSERT_EQ(close(true), 0, "close");
    INFO("finished test");

    return 0;
}

namespace {
void test_get_put() {
    // sync test
    {
        // there is no value
        int64_t get_timestamp = -1;
        int64_t get_policy = -1;
        std::vector<uint8_t> get_value;
        EXPECT_EQ(get((int64_t)3, get_value, get_policy, get_timestamp), true,
                  "get");
        EXPECT_EQ(get_policy, -1, "get");
        EXPECT_EQ(get_timestamp, -1, "get");

        std::vector<uint8_t> put_value;
        put_value.emplace_back(1);
        int64_t put_timestamp = -1;
        int64_t put_policy = -1;
        EXPECT_EQ(put((int64_t)3, put_value, put_policy, put_timestamp), true,
                  "put");
        EXPECT_EQ(put_policy, (int64_t)client_id(), "put");

        EXPECT_EQ(get((int64_t)3, get_value, get_policy, get_timestamp), true,
                  "get");
        EXPECT_EQ(get_timestamp, put_timestamp, "get");
        EXPECT_EQ(get_policy, put_policy, "get");
        EXPECT_EQ(get_value.size(), put_value.size(), "get");
        EXPECT_EQ(get_value, put_value, "get");

        int64_t new_put_timestamp = -1;
        int64_t new_put_policy = -1;
        put_value.emplace_back(2);
        EXPECT_EQ(put((int64_t)3, put_value, new_put_policy, new_put_timestamp),
                  true, "put");
        EXPECT_EQ(new_put_policy, put_policy, "put");
        EXPECT_EQ(new_put_timestamp > put_timestamp, true, "put");

        EXPECT_EQ(get((int64_t)3, get_value, get_policy, get_timestamp), true,
                  "get");
        EXPECT_EQ(get_policy, new_put_policy, "get");
        EXPECT_EQ(get_timestamp, new_put_timestamp, "get");
        EXPECT_EQ(get_value, put_value, "get");
    }

    // async test
    {
        int64_t ticket = get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        auto g_reply = get_reply<
            std::tuple<int64_t, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<2>(g_reply), -1, "get_async");
        EXPECT_EQ(std::get<3>(g_reply), -1, "get_async");

        std::vector<uint8_t> put_value;
        put_value.emplace_back(1);
        ticket = put_async(4, put_value);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for put");
            return;
        }
        auto put_reply =
            get_reply<std::tuple<int64_t, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<1>(put_reply), client_id(), "put_async");

        ticket = get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<
            std::tuple<int64_t, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<1>(g_reply), put_value, "get_async");
        EXPECT_EQ(std::get<2>(g_reply), std::get<1>(put_reply), "get_async");
        EXPECT_EQ(std::get<3>(g_reply), std::get<2>(put_reply), "get_async");

        put_value.emplace_back(2);
        ticket = put_async(4, put_value);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for put");
            return;
        }
        auto new_put_reply =
            get_reply<std::tuple<int64_t, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<1>(new_put_reply), std::get<0>(put_reply),
                  "put_async");
        EXPECT_EQ(std::get<2>(new_put_reply) > std::get<2>(put_reply), true,
                  "put_async");

        ticket = get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<
            std::tuple<int64_t, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<1>(g_reply), put_value, "get_async");
        EXPECT_EQ(std::get<2>(g_reply), std::get<1>(new_put_reply),
                  "get_async");
        EXPECT_EQ(std::get<3>(g_reply), std::get<2>(new_put_reply),
                  "get_async");
    }
}

void test_ping() {
    // sync test
    {
        ping();
        EXPECT_EQ(0, 0, "ping");
    }

    // async test
    {
        int64_t const ticket = ping_async();
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for ping");
            return;
        }
        get_reply<void>(ticket);
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
    std::vector<int64_t> get_tickets;
    std::vector<int64_t> put_tickets;
    std::vector<int64_t> ping_tickets;

    std::vector<uint8_t> value;
    value.emplace_back(5);

    for (int i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0:
                get_tickets.emplace_back(get_async(8));
                break;
            case 1:
                put_tickets.emplace_back(put_async(9, value));
                break;
            case 2:
            default:
                ping_tickets.emplace_back(ping_async());
        }
    }

    while (std::any_of(std::cbegin(get_tickets), std::cend(get_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(put_tickets), std::cend(put_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(ping_tickets), std::cend(ping_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::PENDING;
                       }))
        ;

    for (int64_t const ticket : get_tickets) {
        auto reply = get_reply<
            std::tuple<int64_t, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);
        EXPECT_EQ(std::get<0>(reply), 8, "pipelining");
        EXPECT_EQ(std::get<2>(reply), -1, "pipelining");
    }

    for (int64_t const ticket : put_tickets) {
        auto reply = get_reply<std::pair<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<1>(reply), true, "pipelining");
    }

    for (int64_t const ticket : ping_tickets) {
        get_reply<void>(ticket);
        EXPECT_EQ(0, 0, "pipelining");
    }
}

/**
 * this test tries to verify that the callbacks are being properly executed
 */
void test_cb() {
    int get_n = 0;
    int put_n = 0;
    int change_policy_n = 0;
    int ping_n = 0;
    ASSERT_EQ(get_set_cb([&get_n](int64_t, int64_t, std::vector<uint8_t>,
                                  int64_t, int64_t) { get_n++; }),
              0, "get_set_cb");
    ASSERT_EQ(
        put_set_cb([&put_n](int64_t, int64_t, int64_t, int64_t) { put_n++; }),
        0, "put_set_cb");
    ASSERT_EQ(
        change_policy_set_cb([&change_policy_n](int64_t, int64_t, int64_t) {
            change_policy_n++;
        }),
        0, "change_policy_set_cb");
    ASSERT_EQ(ping_set_cb([&ping_n](int64_t) { ping_n++; }), 0, "ping_set_cb");

    get_cb((int64_t)1);
    ping_cb();
    get_cb((int64_t)4);
    ping_cb();

    std::vector<uint8_t> value;
    value.emplace_back(1);
    put_cb((int64_t)4, value);
    get_cb((int64_t)2);
    get_cb((int64_t)5);
    ping_cb();
    put_cb((int64_t)6, value);

    wait_for();

    EXPECT_EQ(get_n, 4, "callback get test");
    EXPECT_EQ(put_n, 2, "callback put test");
    EXPECT_EQ(ping_n, 3, "callback ping test");
    ASSERT_EQ(get_set_cb([](int64_t, int64_t, std::vector<uint8_t>, int64_t,
                            int64_t) {}),
              0, "get_set_cb");
    ASSERT_EQ(put_set_cb([](int64_t, int64_t, int64_t, int64_t) {}), 0,
              "put_set_cb");
    ASSERT_EQ(change_policy_set_cb([](int64_t, int64_t, int64_t) {}), 0,
              "change_policy_set_cb");
    ASSERT_EQ(ping_set_cb([](int64_t) {}), 0, "ping_set_cb");
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
