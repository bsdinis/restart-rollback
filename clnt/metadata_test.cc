#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "log.h"
#include "metadata.h"
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
using namespace teems;  // namespace sanity

namespace {
void test_metadata_get_put();
void test_ping();
void test_pipelining();

std::string global_config_path = "../server/default.conf";
ssize_t global_index = 0;
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    INFO("starting test");
    ASSERT_EQ(init(global_config_path.c_str(), global_index), 0, "init");

    reset();

    test_ping();
    test_get_put();
    test_metadata_get_put();
    test_pipelining();
    test_cb();

    ASSERT_EQ(close(true), 0, "close");
    INFO("finished test");

    return 0;
}
namespace {
void test_metadata_get_put() {
    // sync test
    {
        // there is no value
        int64_t get_timestamp = -1;
        Metadata get_value;
        EXPECT_EQ(metadata_get((int64_t)3, get_value, get_timestamp), true,
                  "metadata get");
        EXPECT_EQ(get_timestamp, -1, "metadata get");

        Metadata put_value;
        put_value.fill(1);

        int64_t put_timestamp = -1;
        EXPECT_EQ(metadata_put((int64_t)3, put_value, put_timestamp), true,
                  "metadata put");

        EXPECT_EQ(metadata_get((int64_t)3, get_value, get_timestamp), true,
                  "metadata get");
        EXPECT_EQ(get_timestamp, put_timestamp, "metadata get");
        EXPECT_EQ(get_value.size(), put_value.size(), "metadata get");
        EXPECT_EQ(get_value, put_value, "metadata get");

        int64_t new_put_timestamp = -1;
        put_value.fill(2);
        EXPECT_EQ(metadata_put((int64_t)3, put_value, new_put_timestamp), true,
                  "metadata put");
        EXPECT_EQ(new_put_timestamp > put_timestamp, true, "metadata put");

        EXPECT_EQ(metadata_get((int64_t)3, get_value, get_timestamp), true,
                  "metadata get");
        EXPECT_EQ(get_timestamp, new_put_timestamp, "metadata get");
        EXPECT_EQ(get_value, put_value, "metadata get");
    }

    // async test
    {
        int64_t ticket = metadata_get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        auto g_reply =
            get_reply<std::tuple<int64_t, Metadata, int64_t>>(ticket);

        EXPECT_EQ(std::get<2>(g_reply), -1, "metadata get_async");

        Metadata put_value;
        put_value.fill(1);
        ticket = metadata_put_async(4, put_value);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for put");
            return;
        }
        auto put_reply = get_reply<std::pair<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<1>(put_reply), true, "metadata put_async");

        ticket = metadata_get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<std::tuple<int64_t, Metadata, int64_t>>(ticket);

        EXPECT_EQ(std::get<1>(g_reply), put_value, "metadata get_async");
        EXPECT_EQ(std::get<2>(g_reply), std::get<0>(put_reply),
                  "metadata get_async");

        put_value.fill(2);
        ticket = metadata_put_async(4, put_value);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for put");
            return;
        }
        auto new_put_reply = get_reply<std::pair<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<1>(new_put_reply), true, "metadata put_async");
        EXPECT_EQ(std::get<0>(new_put_reply) > std::get<0>(put_reply), true,
                  "metadata put_async");

        ticket = metadata_get_async(4);
        if (wait_for(ticket) == poll_state::ERR) {
            ERROR("failed to wait for get");
            return;
        }
        g_reply = get_reply<std::tuple<int64_t, Metadata, int64_t>>(ticket);

        EXPECT_EQ(std::get<1>(g_reply), put_value, "metadata get_async");
        EXPECT_EQ(std::get<2>(g_reply), std::get<0>(new_put_reply),
                  "metadata get_async");
    }
}

void test_pipelining() {
    std::vector<int64_t> get_tickets;
    std::vector<int64_t> put_tickets;

    Metadata value;
    value.fill(5);

    for (int i = 0; i < 10; i++) {
        switch (i % 2) {
            case 0:
                get_tickets.emplace_back(metadata_get_async(8));
                break;
            case 1:
                put_tickets.emplace_back(metadata_put_async(9, value));
                break;
            default:
                break;
        }
    }

    while (std::any_of(std::cbegin(get_tickets), std::cend(get_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::PENDING;
                       }) ||
           std::any_of(std::cbegin(put_tickets), std::cend(put_tickets),
                       [](int64_t ticket) {
                           return poll(ticket) == poll_state::PENDING;
                       }) ||)
        ;

    for (int64_t const ticket : get_tickets) {
        auto reply = get_reply<std::tuple<int64_t, Metadata, int64_t>>(ticket);
        EXPECT_EQ(std::get<0>(reply), 8, "pipelining");
        EXPECT_EQ(std::get<2>(reply), -1, "pipelining");
    }

    for (int64_t const ticket : put_tickets) {
        auto reply = get_reply<std::pair<int64_t, bool>>(ticket);
        EXPECT_EQ(std::get<1>(reply), true, "pipelining");
    }
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
