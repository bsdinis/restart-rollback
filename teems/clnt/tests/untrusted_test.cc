#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

#include "async.h"
#include "log.h"
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

using namespace teems;  // namespace sanity

namespace {
void test_get_put();
void test_pipelining();

void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("FS test");
    ASSERT_EQ(untrusted_change_store(UntrustedStoreType::Filesystem), 0,
              "change store -> FS");
    test_get_put();
    test_pipelining();

    INFO("Redis test");
    ASSERT_EQ(untrusted_change_store(UntrustedStoreType::Redis), 0,
              "change store -> Redis");
    test_get_put();
    test_pipelining();

    INFO("S3 test");
    ASSERT_EQ(untrusted_change_store(UntrustedStoreType::S3), 0,
              "change store -> S3");
    test_get_put();
    test_pipelining();

    ASSERT_EQ(untrusted_close(), 0, "close");
    INFO("finished test");

    return 0;
}
namespace {
void test_get_put() {
    // sync test
    {
        // there is no value
        std::vector<uint8_t> get_value;
        EXPECT_EQ(untrusted_get(gen_teems_ticket(call_type::Sync), 0, true,
                                std::to_string(3 * getpid()), get_value),
                  false, "get");

        std::vector<uint8_t> put_value(8 << 10, 1);
        EXPECT_EQ(untrusted_put(gen_teems_ticket(call_type::Sync), 0, true,
                                std::to_string(3 * getpid()), put_value),
                  true, "put");

        EXPECT_EQ(untrusted_get(gen_teems_ticket(call_type::Sync), 0, true,
                                std::to_string(3 * getpid()), get_value),
                  true, "get");
        EXPECT_EQ(get_value, put_value, "get");
    }

    // async test
    {
        int64_t ticket =
            untrusted_get_async(gen_teems_ticket(call_type::Sync), 0, true,
                                std::to_string(4 * getpid()));
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        auto g_reply =
            get_reply<std::tuple<bool, std::vector<uint8_t>>>(ticket);
        EXPECT_EQ(std::get<0>(g_reply), false, "get_async");

        std::vector<uint8_t> put_value(8 << 10, 1);
        ticket = untrusted_put_async(gen_teems_ticket(call_type::Sync), 0, true,
                                     std::to_string(4 * getpid()), put_value);
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for put");
            return;
        }
        auto put_reply = get_reply<bool>(ticket);
        EXPECT_EQ(put_reply, true, "put_async");

        ticket = untrusted_get_async(gen_teems_ticket(call_type::Sync), 0, true,
                                     std::to_string(4 * getpid()));
        if (wait_for(ticket) == poll_state::Error) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<std::tuple<bool, std::vector<uint8_t>>>(ticket);

        EXPECT_EQ(std::get<0>(g_reply), true, "get_async");
        EXPECT_EQ(std::get<1>(g_reply), put_value, "get_async");
    }
}

void test_pipelining() {
    std::vector<int64_t> get_tickets;
    std::vector<int64_t> put_tickets;

    std::vector<uint8_t> value(8 << 10, 5);

    for (int i = 0; i < 10; i++) {
        switch (i % 2) {
            case 0:
                get_tickets.emplace_back(untrusted_get_async(
                    gen_teems_ticket(call_type::Sync), 0, true,
                    std::to_string(8 * getpid() + i)));
                break;
            case 1:
                put_tickets.emplace_back(untrusted_put_async(
                    gen_teems_ticket(call_type::Sync), 0, true,
                    std::to_string(8 * getpid() + i), value));
                break;
            default:
                break;
        }
    }

    while (std::any_of(std::cbegin(get_tickets), std::cend(get_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }) ||
           std::any_of(std::cbegin(put_tickets), std::cend(put_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::Pending;
                       }))
        ;

    for (int64_t const ticket : get_tickets) {
        auto reply = get_reply<std::tuple<bool, std::vector<uint8_t>>>(ticket);
    }

    for (int64_t const ticket : put_tickets) {
        auto reply = get_reply<bool>(ticket);
        EXPECT_EQ(reply, true, "pipelining");
    }
}

void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:i:h")) != -1) {
        switch (opt) {
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
