#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "cache.h"
#include "log.h"
#include "teems.h"
#include "untrusted.h"

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

using namespace teems;

namespace {
void test_get_put();
void test_change_policy();
void test_ping();
void test_pipelining();
void test_cb();
void test_cache();

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

void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("starting test");
    ASSERT_EQ(init(g_config_path.c_str(), g_storage_type, g_name_cache_size,
                   g_value_cache_size),
              0, "init");

    reset();

    test_ping();
    test_get_put();
    test_change_policy();
    test_pipelining();
    test_cb();
    test_cache();

    ASSERT_EQ(close(true), 0, "close");
    INFO("finished test");

    return 0;
}

namespace {
void test_get_put() {
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
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        auto g_reply = get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<3>(g_reply), -1, "get_async");
        EXPECT_EQ(std::get<4>(g_reply), -1, "get_async");

        std::vector<uint8_t> put_value;
        put_value.emplace_back(1);
        ticket = put_async(4, put_value);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for put");
            return;
        }
        auto put_reply =
            get_reply<std::tuple<int64_t, bool, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<2>(put_reply), client_id(), "put_async");

        ticket = get_async(4);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<2>(g_reply), put_value, "get_async");
        EXPECT_EQ(std::get<3>(g_reply), std::get<2>(put_reply), "get_async");
        EXPECT_EQ(std::get<4>(g_reply), std::get<3>(put_reply), "get_async");

        put_value.emplace_back(2);
        ticket = put_async(4, put_value);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for put");
            return;
        }
        auto new_put_reply =
            get_reply<std::tuple<int64_t, bool, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<2>(new_put_reply), std::get<2>(put_reply),
                  "put_async");
        EXPECT_EQ(std::get<3>(new_put_reply) > std::get<3>(put_reply), true,
                  "put_async");

        ticket = get_async(4);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<2>(g_reply), put_value, "get_async");
        EXPECT_EQ(std::get<3>(g_reply), std::get<2>(new_put_reply),
                  "get_async");
        EXPECT_EQ(std::get<4>(g_reply), std::get<3>(new_put_reply),
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
        if (wait_for(ticket) == poll_state::Error) {
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
    std::vector<int64_t> change_policy_tickets;
    std::vector<int64_t> ping_tickets;

    std::vector<uint8_t> value;
    value.emplace_back(5);

    // create the value
    int64_t policy_version;
    int64_t timestamp;
    ASSERT_EQ(put(9, value, policy_version, timestamp), true,
              "pipeline prepare");

    for (int i = 0; i < 20; i++) {
        switch (i % 4) {
            case 0:
                get_tickets.emplace_back(get_async(9));
                break;
            case 1:
                put_tickets.emplace_back(put_async(9, value));
                break;
            case 2:
                change_policy_tickets.emplace_back(
                    change_policy_async(9, 0xff));
            case 3:
            default:
                ping_tickets.emplace_back(ping_async());
        }
    }

    while (std::any_of(std::cbegin(get_tickets), std::cend(get_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }) ||
           std::any_of(std::cbegin(put_tickets), std::cend(put_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }) ||
           std::any_of(std::cbegin(change_policy_tickets),
                       std::cend(change_policy_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }) ||
           std::any_of(std::cbegin(ping_tickets), std::cend(ping_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }))
        ;

    for (int64_t const ticket : get_tickets) {
        auto reply = get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);
        EXPECT_EQ(std::get<0>(reply), 9, "pipelining get");
        EXPECT_EQ(std::get<1>(reply), true, "pipelining get");
    }

    for (int64_t const ticket : put_tickets) {
        auto reply =
            get_reply<std::tuple<int64_t, bool, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<1>(reply), true, "pipelining put");
    }

    for (int64_t const ticket : change_policy_tickets) {
        auto reply = get_reply<std::tuple<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<1>(reply), true, "pipelining change policy");
    }

    for (int64_t const ticket : ping_tickets) {
        get_reply<void>(ticket);
        EXPECT_EQ(0, 0, "pipelining ping");
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
    ASSERT_EQ(get_set_cb([&get_n](int64_t, int64_t, bool, std::vector<uint8_t>,
                                  int64_t, int64_t) { get_n++; }),
              0, "get_set_cb");
    ASSERT_EQ(put_set_cb([&put_n](int64_t, int64_t, bool, int64_t, int64_t) {
                  put_n++;
              }),
              0, "put_set_cb");
    ASSERT_EQ(change_policy_set_cb(
                  [&change_policy_n](int64_t, int64_t, bool, int64_t) {
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
    ASSERT_EQ(get_set_cb([](int64_t, int64_t, bool, std::vector<uint8_t>,
                            int64_t, int64_t) {}),
              0, "get_set_cb");
    ASSERT_EQ(put_set_cb([](int64_t, int64_t, bool, int64_t, int64_t) {}), 0,
              "put_set_cb");
    ASSERT_EQ(change_policy_set_cb([](int64_t, int64_t, bool, int64_t) {}), 0,
              "change_policy_set_cb");
    ASSERT_EQ(ping_set_cb([](int64_t) {}), 0, "ping_set_cb");
}

void test_change_policy() {
    // sync test
    {
        // there is no value
        int64_t policy_version = -1;
        EXPECT_EQ(change_policy(13, 0xff, policy_version), false,
                  "change policy");
        EXPECT_EQ(policy_version, -1, "change policy");

        std::vector<uint8_t> put_value;
        put_value.emplace_back(0);
        int64_t put_policy_version = -1;
        int64_t put_timestamp = -1;
        EXPECT_EQ(put(13, put_value, put_policy_version, put_timestamp), true,
                  "put");

        EXPECT_EQ(change_policy(13, 0xff, policy_version), true,
                  "change policy");
        EXPECT_EQ(policy_version, put_policy_version + 1, "change policy");

        std::vector<uint8_t> get_value;
        int64_t get_policy_version = -1;
        int64_t get_timestamp = -1;
        EXPECT_EQ(get(13, get_value, get_policy_version, get_timestamp), true,
                  "get");
        EXPECT_EQ(get_policy_version, policy_version, "get");
        EXPECT_EQ(get_timestamp, put_timestamp, "get");
    }

    // async test
    {
        int64_t ticket = change_policy_async(14, 0xff);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for change policy");
            return;
        }

        auto change_policy_reply = get_reply<std::tuple<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<0>(change_policy_reply), -1, "change policy async");
        EXPECT_EQ(std::get<1>(change_policy_reply), false,
                  "change policy async");

        std::vector<uint8_t> put_value;
        put_value.emplace_back(0);
        ticket = put_async(14, put_value);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for put");
            return;
        }
        auto put_reply =
            get_reply<std::tuple<int64_t, bool, int64_t, int64_t>>(ticket);
        EXPECT_EQ(std::get<1>(put_reply), true, "put async");

        ticket = change_policy_async(14, 0xff);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }

        change_policy_reply = get_reply<std::tuple<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<0>(change_policy_reply), std::get<2>(put_reply) + 1,
                  "change policy async");
        EXPECT_EQ(std::get<1>(change_policy_reply), true,
                  "change policy async");

        ticket = get_async(14);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        auto g_reply = get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);

        EXPECT_EQ(std::get<3>(g_reply), std::get<0>(change_policy_reply),
                  "get async");
        EXPECT_EQ(std::get<4>(g_reply), std::get<3>(put_reply), "get async");
    }
}

void cache_writing_workload() {
    std::vector<uint8_t> put_value;
    put_value.emplace_back(1);
    int64_t put_timestamp = -1;
    int64_t put_policy = -1;
    EXPECT_EQ(put((int64_t)100, put_value, put_policy, put_timestamp), true,
              "cache writing workload");
    EXPECT_EQ(put_policy, (int64_t)client_id(), "cache writing workload");

    int64_t get_timestamp = -1;
    int64_t get_policy = -1;
    std::vector<uint8_t> get_value;
    EXPECT_EQ(get((int64_t)100, get_value, get_policy, get_timestamp), true,
              "cache writing workload");
    EXPECT_EQ(get_timestamp, put_timestamp, "cache writing workload");
    EXPECT_EQ(get_policy, put_policy, "cache writing workload");
    EXPECT_EQ(get_value.size(), put_value.size(), "cache writing workload");
    EXPECT_EQ(get_value, put_value, "cache writing workload");
}

void cache_reading_workload(int64_t policy, int64_t timestamp,
                            std::vector<uint8_t> const& value) {
    int64_t get_timestamp = -1;
    int64_t get_policy = -1;
    std::vector<uint8_t> get_value;

    EXPECT_EQ(get((int64_t)101, get_value, get_policy, get_timestamp), true,
              "cache reading workload");
    EXPECT_EQ(get_timestamp, timestamp, "cache reading workload");
    EXPECT_EQ(get_policy, policy, "cache reading workload");
    EXPECT_EQ(get_value.size(), value.size(), "cache reading workload");
    EXPECT_EQ(get_value, value, "cache reading workload");

    EXPECT_EQ(get((int64_t)101, get_value, get_policy, get_timestamp), true,
              "cache reading workload");
    EXPECT_EQ(get_timestamp, timestamp, "cache reading workload");
    EXPECT_EQ(get_policy, policy, "cache reading workload");
    EXPECT_EQ(get_value.size(), value.size(), "cache reading workload");
    EXPECT_EQ(get_value, value, "cache reading workload");
}

void test_cache() {
    std::vector<uint8_t> put_value;
    put_value.emplace_back(1);
    int64_t put_timestamp = -1;
    int64_t put_policy = -1;
    EXPECT_EQ(put((int64_t)101, put_value, put_policy, put_timestamp), true,
              "put");
    EXPECT_EQ(put_policy, (int64_t)client_id(), "put");

    reset_name_cache(0);
    reset_value_cache(0);

    cache_writing_workload();
    cache_reading_workload(put_policy, put_timestamp, put_value);

    EXPECT_EQ(name_hits(), 0, "no name cache => no hits");
    EXPECT_EQ(name_misses(), 3, "no name cache => all miss");
    EXPECT_EQ(value_hits(), 0, "no value cache => no hits");
    EXPECT_EQ(value_misses(), 3, "no value cache => all miss");

    reset_name_cache(2);
    reset_value_cache(0);

    cache_writing_workload();
    cache_reading_workload(put_policy, put_timestamp, put_value);

    EXPECT_EQ(name_hits(), 2, "name cache => hits");
    EXPECT_EQ(name_misses(), 1, "name cache => no hot miss");
    EXPECT_EQ(value_hits(), 0, "no value cache => no hits");
    EXPECT_EQ(value_misses(), 3, "no value cache => all miss");

    reset_name_cache(2);
    reset_value_cache(1 << 10);

    cache_writing_workload();
    cache_reading_workload(put_policy, put_timestamp, put_value);

    EXPECT_EQ(name_hits(), 0, "value cache => no name cache hits");
    EXPECT_EQ(name_misses(), 1, "value cache => only cold miss");
    EXPECT_EQ(value_hits(), 2, "value cache => hits");
    EXPECT_EQ(value_misses(), 1, "value cache => only cold miss");
}

// cli
void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n", g_config_path.c_str());
    fprintf(stderr, "\t-h : print help message\n");
}

void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:i:a:b:s:h")) != -1) {
        switch (opt) {
            case 'c':
                g_config_path = std::string(optarg, strlen(optarg));
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

}  // anonymous namespace
