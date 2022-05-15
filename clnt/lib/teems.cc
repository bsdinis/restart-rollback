/**
 * teems.c
 */

#include "teems.h"

#include "async.h"
#include "cache.h"
#include "context.h"
#include "log.h"
#include "lru.h"
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

//==============================
// HELPERS
//==============================
namespace {
int64_t get_non_sync(int64_t key, call_type type);
int64_t put_non_sync(int64_t key, std::vector<uint8_t> const &value,
                     call_type type);
int64_t change_policy_non_sync(int64_t key, uint8_t policy, call_type type);
}  // anonymous namespace

extern std::tuple<int64_t, Metadata, int64_t> g_metadata_get_result;
extern std::pair<int64_t, bool> g_metadata_put_result;

// stores name hints for TEEMS
extern LRUCache<int64_t, std::string> g_name_cache;

// stores value hints for TEEMS
extern LRUCache<int64_t, std::pair<std::string, std::vector<uint8_t>>>
    g_value_cache;

// protocol globals

int32_t g_client_id = -1;

// callbacks
std::function<void(int64_t, int64_t, bool, std::vector<uint8_t>, int64_t,
                   int64_t)>
    g_get_callback =
        [](int64_t, int64_t, bool, std::vector<uint8_t>, int64_t, int64_t) {};
std::function<void(int64_t, int64_t, bool, int64_t, int64_t)> g_put_callback =
    [](int64_t, int64_t, bool, int64_t, int64_t) {};
std::function<void(int64_t, int64_t, bool, int64_t)> g_change_policy_callback =
    [](int64_t, int64_t, bool, int64_t) {};
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
    char const *config,             // config file
    UntrustedStoreType ustor_type,  // type of untrusted storage
    size_t name_cache_size,   // size (in number of names) of the name cache
    size_t value_cache_size,  // size (in number of bytes) of the value cache
    size_t concurrent_hint,   // hint for number of concurrent calls permitted
    struct timeval timeout,   // timeout for select
    char const *cert_path,    // certificate for the client
    char const *key_path      // private key of the certificate
) {
    g_timeout = timeout;

    reset_name_cache(name_cache_size);
    reset_value_cache(value_cache_size);
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
    int64_t const ticket = get_non_sync(key, call_type::Async);
    if (wait_for(ticket) == poll_state::Error) {
        ERROR("get(%ld): failed to wait for result", key);
        return false;
    }

    bool success;
    std::tie(std::ignore, success, value, policy_version, timestamp) =
        get_reply<
            std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
            ticket);
    return success;
}

bool put(int64_t key, std::vector<uint8_t> const &value,
         int64_t &policy_version, int64_t &timestamp) {
    int64_t const ticket = put_non_sync(key, value, call_type::Async);
    if (wait_for(ticket) == poll_state::Error) {
        ERROR("put(%ld): failed to wait for result", key);
        return false;
    }

    bool success;
    std::tie(std::ignore, success, policy_version, timestamp) =
        get_reply<std::tuple<int64_t, bool, int64_t, int64_t>>(ticket);
    return success;
}
bool change_policy(int64_t key, uint8_t policy, int64_t &policy_version) {
    return metadata_change_policy(gen_teems_ticket(call_type::Sync), 0, false,
                                  key, policy, policy_version);
}
void ping() {
    int64_t const ticket = send_ping_request(g_servers[0], call_type::Sync);

    if (ticket == -1) {
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
        int64_t const ticket = send_reset_request(server, call_type::Sync);
        if (ticket == -1) {
            ERROR("Failed to reset");
            return;
        }
        if (block_until_return(idx, server) == -1) {
            ERROR("failed to get a return from the server");
        }
    }
}

// async api
int64_t get_async(int64_t key) { return get_non_sync(key, call_type::Async); }
int64_t put_async(int64_t key, std::vector<uint8_t> const &value) {
    return put_non_sync(key, value, call_type::Async);
}
int64_t change_policy_async(int64_t key, uint8_t policy) {
    return change_policy_non_sync(key, policy, call_type::Async);
}
int64_t ping_async() {
    return send_ping_request(g_servers[0], call_type::Async);
}
int64_t reset_async() {
    return send_reset_request(g_servers[0], call_type::Async);
}

// callback api
int get_set_cb(std::function<void(int64_t, int64_t, bool, std::vector<uint8_t>,
                                  int64_t, int64_t)>
                   cb) {
    g_get_callback = cb;
    return 0;
}
int64_t get_cb(int64_t key) { return get_non_sync(key, call_type::Callback); }

int put_set_cb(
    std::function<void(int64_t, int64_t, bool, int64_t, int64_t)> cb) {
    g_put_callback = cb;
    return 0;
}
int64_t put_cb(int64_t key, std::vector<uint8_t> const &value) {
    return put_non_sync(key, value, call_type::Callback);
}

int change_policy_set_cb(
    std::function<void(int64_t, int64_t, bool, int64_t)> cb) {
    g_change_policy_callback = cb;
    return 0;
}
int64_t change_policy_cb(int64_t key, uint8_t policy) {
    return change_policy_non_sync(key, policy, call_type::Callback);
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

namespace {
int64_t get_non_sync(int64_t key, call_type type) {
    int64_t const ticket = gen_teems_ticket(type);
    issued_call(ticket);

    std::string name_hint;
    std::vector<uint8_t> value_hint;
    auto const *v_hint = g_value_cache.get(key);
    std::string const *n_hint = nullptr;
    if (v_hint == nullptr) {
        n_hint = g_name_cache.get(key);
        if (n_hint != nullptr) {
            name_hint = *n_hint;
        }
    } else {
        name_hint = v_hint->first;
        value_hint = v_hint->second;
    }

    int64_t const sub_ticket =
        gen_metadata_ticket(ticket, 0, false, call_type::Async);
    if (add_get_call(sub_ticket, key, name_hint,
                     (v_hint == nullptr) ? nullptr : &value_hint) == -1) {
        ERROR("get(%ld): failed to add call context", key);
        finished_call(ticket);
        return -1;
    }

    if (metadata_get_async(ticket, 0, false, key) == -1) {
        ERROR("get(%ld): failed to start metadata read", key);
        rem_get_call(ticket);
        finished_call(ticket);
        return -1;
    }

    if (n_hint != nullptr) {
        // we don't have the value but we have a name hint
        if (untrusted_get_async(ticket, 0, false, name_hint) == -1) {
            ERROR("get(%ld): failed to start untrusted read", key);
            return -1;
        }
    }

    return ticket;
}
int64_t put_non_sync(int64_t key, std::vector<uint8_t> const &value,
                     call_type type) {
    int64_t const ticket = gen_teems_ticket(type);
    issued_call(ticket);

    Metadata metadata;
    std::vector<uint8_t> encrypted_value;
    if (metadata.encrypt_value(value, encrypted_value) == false) {
        ERROR("put(%ld): failed to encrypt value", key);
        finished_call(ticket);
        return false;
    }

    int64_t const sub_ticket =
        gen_untrusted_ticket(ticket, 0, false, call_type::Async);
    if (add_put_call(sub_ticket, key, std::move(metadata), value) == -1) {
        ERROR("put(%ld): failed to add call context", key);
        finished_call(ticket);
        return -1;
    }

    std::string const ustor_name = metadata.ustor_name();
    if (untrusted_put_async(ticket, 0, false, ustor_name, encrypted_value) ==
        -1) {
        ERROR(
            "put(%ld): failed to start write encrypted value to untrusted "
            "storage "
            "under %s",
            key, ustor_name.c_str());
        rem_put_call(ticket);
        finished_call(ticket);
        return -1;
    }

    return ticket;
}

int64_t change_policy_non_sync(int64_t key, uint8_t policy, call_type type) {
    int64_t const super_ticket = gen_teems_ticket(type);
    int64_t const ticket =
        metadata_change_policy_async(super_ticket, 0, false, key, policy);

    if (ticket == -1) {
        ERROR("change policy(%ld): failed to start change policy", key);
        finished_call(ticket);
        return -1;
    }

    return ticket;
}
}  // anonymous namespace
}  // namespace teems
