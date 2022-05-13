/**
 * teems.c
 */

#include "teems.h"

#include "async.h"
#include "log.h"
#include "metadata.h"
#include "network.h"
#include "protocol_helpers.h"
#include "result.h"
#include "ssl_util.h"
#include "teems_config.h"
#include "teems_generated.h"
#include "untrusted.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <numeric>
#include <unordered_map>

namespace teems {

using FBValue = flatbuffers::Array<uint8_t, REGISTER_SIZE>;

extern std::tuple<int64_t, Metadata, int64_t> g_metadata_get_result;
extern std::pair<int64_t, bool> g_metadata_put_result;

// protocol globals

int32_t g_client_id = -1;

// callbacks
std::function<void(int64_t, int64_t, std::vector<uint8_t>, int64_t, int64_t)>
    g_get_callback =
        [](int64_t, int64_t, std::vector<uint8_t>, int64_t, int64_t) {};
std::function<void(int64_t, int64_t, int64_t, int64_t)> g_put_callback =
    [](int64_t, int64_t, int64_t, int64_t) {};
std::function<void(int64_t, int64_t, int64_t)> g_change_policy_callback =
    [](int64_t, int64_t, int64_t) {};
std::function<void(int64_t)> g_ping_callback = [](int64_t) {};
std::function<void(int64_t)> g_reset_callback = [](int64_t) {};

// connection globals
SSL_CTX *g_client_ctx = nullptr;
timeval g_timeout;
::std::vector<peer> g_servers;

// =================================
// public implementation
// =================================
int init(
    char const *config,      // config file
    size_t concurrent_hint,  // hint for number of concurrent calls permitted
    struct timeval timeout,  // timeout for select
    char const *cert_path,   // certificate for the client
    char const *key_path,    // private key of the certificate
    UntrustedStoreType ustor_type  // type of untrusted storage
) {
    g_timeout = timeout;

    if (async_init(concurrent_hint) != 0) {
        ERROR("failed to initialize the async context");
        return -1;
    }

    if (metadata_init(config, cert_path, key_path) != 0) {
        ERROR("failed to initialize the metadata subsystem");
        return -1;
    }

    if (untrusted_change_store(ustor_type) != 0) {
        ERROR("failed to initialize the untrusted storage subsystem");
        return -1;
    }

    return 0;
}

int close(bool close_remote) {
    if (metadata_close(close_remote) != 0) {
        ERROR("failed to close metadata subsystem");
        return -1;
    }

    if (untrusted_close() != 0) {
        ERROR("failed to close untrusted subsystem");
        return -1;
    }

    if (async_close() != 0) {
        ERROR("failed to close async context");
        return -1;
    }

    return 0;
}

int32_t client_id() { return g_client_id; }

// sync api
bool get(int64_t key, std::vector<uint8_t> &value, int64_t &policy_version,
         int64_t &timestamp) {
    // TODO
    return true;
}
bool put(int64_t key, std::vector<uint8_t> const &value,
         int64_t &policy_version, int64_t &timestamp) {
    // TODO
    return true;
}
bool change_policy(int64_t key, uint64_t policy, int64_t &policy_version) {
    // TODO
    return true;
}
void ping() {
    if (send_ping_request(g_servers[0], call_type::Sync) == -1) {
        ERROR("Failed to ping");
        return;
    }

    if (block_until_return(0, g_servers[0]) == -1) {
        ERROR("failed to get a return from the basicserver");
    }
}

void reset() {
    for (size_t idx = 0; idx < g_servers.size(); ++idx) {
        auto &server = g_servers[idx];
        if (send_reset_request(server, call_type::Sync) == -1) {
            ERROR("Failed to reset");
            return;
        }
        if (block_until_return(idx, server) == -1) {
            ERROR("failed to get a return from the basicserver");
        }
    }
}

// async api
int64_t get_async(int64_t key) {
    // TODO
    return -1;
}
int64_t put_async(int64_t key, std::vector<uint8_t> const &value) {
    // TODO
    return -1;
}
int64_t change_policy_async(int64_t key, uint64_t policy) {
    // TODO
    return -1;
}
int64_t ping_async() {
    return send_ping_request(g_servers[0], call_type::Async);
}
int64_t reset_async() {
    return send_reset_request(g_servers[0], call_type::Async);
}

// callback api
int get_set_cb(std::function<void(int64_t, int64_t, std::vector<uint8_t>,
                                  int64_t, int64_t)>
                   cb) {
    g_get_callback = cb;
    return 0;
}
int64_t get_cb(int64_t key) {
    // TODO
    return -1;
}

int put_set_cb(std::function<void(int64_t, int64_t, int64_t, int64_t)> cb) {
    g_put_callback = cb;
    return 0;
}
int64_t put_cb(int64_t key, std::vector<uint8_t> const &value) {
    // TODO
    return -1;
}

int change_policy_set_cb(std::function<void(int64_t, int64_t, int64_t)> cb) {
    g_change_policy_callback = cb;
    return 0;
}
int64_t change_policy_cb(int64_t key, uint64_t policy) {
    // TODO
    return -1;
}

int ping_set_cb(std::function<void(int64_t)> cb) {
    g_ping_callback = cb;
    return 0;
}
int64_t ping_cb() {
    return send_ping_request(g_servers[0], call_type::Callback);
}

int reset_set_cb(std::function<void(int64_t)> cb) {
    g_reset_callback = cb;
    return 0;
}
int64_t reset_cb() {
    return send_reset_request(g_servers[0], call_type::Callback);
}

}  // namespace teems
