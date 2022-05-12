/**
 * teems.c
 */

#include "teems.h"

#include "config.h"
#include "log.h"
#include "metadata.h"
#include "network.h"
#include "protocol_helpers.h"
#include "result.h"
#include "ssl_util.h"
#include "teems_config.h"
#include "teems_generated.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <numeric>
#include <unordered_map>

namespace teems {

using FBValue = flatbuffers::Array<uint8_t, REGISTER_SIZE>;

extern ::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;

extern std::tuple<int64_t, Metadata, int64_t> g_metadata_get_result;
extern std::pair<int64_t, bool> g_metadata_put_result;

// protocol globals

int32_t g_client_id = -1;

// ticket of the last call to be made
int64_t g_call_ticket = 0;
size_t g_calls_issued = 0;
size_t g_calls_concluded = 0;

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
    char const *key_path     // private key of the certificate
) {
    g_timeout = timeout;
    config_t conf;
    if (config_parse(&conf, config) == -1) {
        ERROR("failed to stat configuration: %s", config);
        return -1;
    }

    g_servers.reserve(conf.size);

    if (init_client_ssl_ctx(&g_client_ctx) == -1) {
        ERROR("failed to setup ssl ctx");
        return -1;
    }

    if (load_certificates(g_client_ctx, cert_path, key_path) == -1) {
        ERROR("failed load certs");
        return -1;
    }

    for (ssize_t idx = 0; idx < conf.size; ++idx) {
        auto const &peer_node = conf.nodes[idx];
        if (connect_to_proxy(peer_node) == -1) {
            ERROR("failed to connect to QP on %s:%d", peer_node.addr,
                  peer_node.port);
            config_free(&conf);
            close_ssl_ctx(g_client_ctx);
            return -1;
        }

        INFO("connected to QP on %s:%d", peer_node.addr, peer_node.port);
    }

    config_free(&conf);
    g_results_map.reserve(concurrent_hint);

    while (g_client_id == -1) {
        struct timeval timeout = g_timeout;
        auto res = process_peers(&timeout);
        if (res == process_res::ERR) {
            return false;
        }
    }
    return 0;
}

int close(bool close_remote) {
    // this will close the remote QP
    if (close_remote) {
        int64_t const ticket = gen_ticket(call_type::SYNC);
        flatbuffers::FlatBufferBuilder builder;
        auto close_args = teems::CreateEmpty(builder);
        auto request =
            teems::CreateMessage(builder, teems::MessageType_close_req, ticket,
                                 teems::BasicMessage_Empty, close_args.Union());
        builder.Finish(request);

        size_t const size = builder.GetSize();
        uint8_t const *payload = builder.GetBufferPointer();

        for (size_t idx = 0; idx < g_servers.size(); ++idx) {
            auto &server = g_servers[idx];
            if (server.append(&size, 1) == -1) {
                // encode message header
                ERROR("failed to prepare message to send");
                return -1;
            }
            if (server.append(payload, size) ==
                -1) {  // then the segment itself
                ERROR("failed to prepare message to send");
                return -1;
            }

            server.flush();
            process_peer(idx, server, nullptr);  // block, gets released by EOF
        }
    }

    for (auto &server : g_servers) {
        server.close();
    }

    close_ssl_ctx(g_client_ctx);
    g_servers.clear();
    return 0;
}

size_t n_calls_issued() { return g_calls_issued; }
size_t n_calls_concluded() { return g_calls_concluded; }
size_t n_calls_outlasting() { return n_calls_issued() - n_calls_concluded(); }
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
    if (send_ping_request(g_servers[0], call_type::SYNC) == -1) {
        ERROR("Failed to ping");
        return;
    }

    if (block_until_return(0, g_servers[0]) == -1) {
        ERROR("failed to get a return from the basicQP");
    }
}

void reset() {
    for (size_t idx = 0; idx < g_servers.size(); ++idx) {
        auto &server = g_servers[idx];
        if (send_reset_request(server, call_type::SYNC) == -1) {
            ERROR("Failed to reset");
            return;
        }
        if (block_until_return(idx, server) == -1) {
            ERROR("failed to get a return from the basicQP");
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
    return send_ping_request(g_servers[0], call_type::ASYNC);
}
int64_t reset_async() {
    return send_reset_request(g_servers[0], call_type::ASYNC);
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
    return send_ping_request(g_servers[0], call_type::CALLBACK);
}

int reset_set_cb(std::function<void(int64_t)> cb) {
    g_reset_callback = cb;
    return 0;
}
int64_t reset_cb() {
    return send_reset_request(g_servers[0], call_type::CALLBACK);
}

// functions to advance state
poll_state poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) return poll_state::READY;
    if (n_calls_outlasting() == 0) return poll_state::NO_CALLS;

    if (!g_servers[0].connected()) {
        ERROR("no connection to proxy");
        return poll_state::ERR;
    }
    if (std::any_of(std::cbegin(g_servers), std::cend(g_servers),
                    [](peer const &server) { return !server.connected(); })) {
        ERROR("no connection to proxies");
        return poll_state::ERR;
    }

    struct timeval timeout = g_timeout;
    auto const res = process_peer(0, g_servers[0], &timeout);
    switch (res) {
        case process_res::ERR:
            ERROR("connection broke");
            return poll_state::ERR;

        case process_res::HANDLED_MSG:
            return (ticket == -1 || has_result(ticket)) ? poll_state::READY
                                                        : poll_state::PENDING;

        case process_res::NOOP:
            return poll_state::PENDING;
    }
    return n_calls_outlasting() == 0 ? poll_state::NO_CALLS
                                     : poll_state::PENDING;
}

poll_state wait_for(int64_t ticket) {
    if (ticket == -1) {
        poll_state res = poll_state::PENDING;
        while (res != poll_state::NO_CALLS && res != poll_state::ERR)
            res = poll();

        return res;
    } else {
        while (poll(ticket) == poll_state::PENDING)
            ;
        if (poll(ticket) == poll_state::ERR) return poll_state::ERR;
        return poll_state::READY;
    }
}

template <typename T>
T get_reply(int64_t ticket) {
    auto it = g_results_map.find(ticket);
    if (it == std::end(g_results_map)) {
        ERROR("failed to find reply for %ld", ticket);
        return T();
    }
    std::unique_ptr<result> res = std::move(it->second);  // mv constructor
    g_results_map.erase(it);

    if (res->type() != result_type::OneVal) {
        ERROR(
            "the default template implementation of %s() only works for OneVal "
            "results",
            __func__);
        ERROR("returning default constructed value");
        return T();
    }

    one_val_result<T> *downcasted =
        dynamic_cast<one_val_result<T> *>(res.get());
    return downcasted->get();
}

// for put
template std::tuple<int64_t, int64_t, int64_t> get_reply(int64_t ticket);

// for get
template std::tuple<int64_t, std::vector<uint8_t>, int64_t, int64_t> get_reply(
    int64_t ticket);

// for metadata put
template std::pair<int64_t, bool> get_reply(int64_t ticket);

// for metadata get
template std::tuple<int64_t, Metadata, int64_t> get_reply(int64_t ticket);

// for ping reset
template <>
void get_reply(int64_t ticket) {
    auto it = g_results_map.find(ticket);
    if (it == std::end(g_results_map)) {
        ERROR("failed to find reply for %ld", ticket);
        return;
    }

    std::unique_ptr<result> res = std::move(it->second);
    if (res->type() != result_type::None)
        ERROR(
            "the template specialization of %s for None was called with "
            "another value",
            __func__);
    g_results_map.erase(it);
}

// eg: if you are adding an RPC like `double avg(std::vector<int64_t> & const
// v)` you should add the following line `template double get_reply(int64_t
// double)`

}  // namespace teems
