/**
 * qp_clnt.c
 */

#include "qp_clnt.h"
#include "fast_get_context.h"
#include "log.h"
#include "qp_result.h"

#include "config.h"

#include "peer.h"
#include "ssl_util.h"

#include "crash_generated.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <numeric>
#include <unordered_map>

namespace paxos_sgx {
namespace crash {

namespace {
// protocol helpers
enum class call_type { SYNC, ASYNC, CALLBACK };
// fast
int64_t send_fast_get_request(int64_t account, call_type type);
int fast_get_handler(int64_t ticket, int64_t amount, int64_t last_applied,
                     int64_t last_accepted);
int fast_get_handler_sync(int64_t ticket, int64_t amount, int64_t last_applied,
                          int64_t last_accepted);
int fast_get_handler_async(int64_t ticket, int64_t amount, int64_t last_applied,
                           int64_t last_accepted);
int fast_get_handler_cb(int64_t ticket, int64_t amount, int64_t last_applied,
                        int64_t last_accepted);
std::function<void(int64_t, int64_t, bool)> fast_get_callback =
    [](int64_t, int64_t, bool) {};

// transfer
int64_t send_transfer_request(peer &server, int64_t account, int64_t to,
                              int64_t amount, call_type type);
int transfer_handler(int64_t ticket, int64_t result, bool success);
int transfer_handler_sync(int64_t transfer, bool success);
int transfer_handler_async(int64_t ticket, int64_t transfer, bool success);
std::function<void(int64_t, int64_t, bool)> transfer_callback =
    [](int64_t, int64_t, bool) {};

// ping
int64_t send_ping_request(peer &server, call_type type);
int ping_handler(int64_t ticket);
int ping_handler_sync();
int ping_handler_async(int64_t ticket);
std::function<void(int64_t)> ping_callback = [](int64_t) {};

// reset
int64_t send_reset_request(peer &server, call_type type);
int reset_handler(int64_t ticket);
int reset_handler_sync();
int reset_handler_async(int64_t ticket);
std::function<void(int64_t)> reset_callback = [](int64_t) {};

// protocol globals
// ticket of the last call to be made
int64_t call_ticket = 0;
size_t calls_issued = 0;
size_t calls_concluded = 0;

// ticket numbers have semantic.
// specifically, the remainder mod 3 indicates
// which type of call they relate to
inline int64_t gen_ticket(call_type type) {
    int const target =
        type == call_type::SYNC ? 0 : (type == call_type::ASYNC ? 1 : 2);
    call_ticket += 3 - (call_ticket % 3) + target;
    return call_ticket;
}

// state of fast get calls
std::unordered_map<int64_t, FastGetContext> g_fast_get_ctx_map;
FastGetContext g_fast_get_sync_ctx;

// global variables to channel the results of synchronous calls to
int64_t fast_get_amount_result;
bool fast_get_success_result;
int64_t fast_get_sync_ticket = -1;

int64_t transfer_amount_result;
bool transfer_success_result;
int64_t transfer_sync_ticket = -1;

// async results
std::unordered_map<int64_t, std::unique_ptr<result>> results_map;
bool has_result(int64_t ticket);

// network globals
SSL_CTX *client_ctx = nullptr;
std::vector<peer> g_servers;
timeval global_timeout;

inline size_t quorum_size() { return ((g_servers.size() - 1) / 2) + 1; }

// network helpers
enum class process_res { HANDLED_MSG, NOOP, ERR };
int handle_received_message(peer &p);
int build_fd_sets(peer const &server, fd_set *read_fds, fd_set *write_fds,
                  fd_set *except_fds);
int build_all_fd_sets(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);
int block_until_return(peer &server);
int connect_to_proxy(config_node_t const &peer_node);
process_res process_peer(peer &p, struct timeval *timeout = nullptr);
process_res process_peers(struct timeval *timeout = nullptr);
}  // namespace

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
    global_timeout = timeout;
    config_t conf;
    if (config_parse(&conf, config) == -1) {
        ERROR("failed to stat configuration: %s", config);
        return -1;
    }

    g_servers.reserve(conf.size);

    if (init_client_ssl_ctx(&client_ctx) == -1) {
        ERROR("failed to setup ssl ctx");
        return -1;
    }

    if (load_certificates(client_ctx, cert_path, key_path) == -1) {
        ERROR("failed load certs");
        return -1;
    }

    for (ssize_t idx = 0; idx < conf.size; ++idx) {
        auto const &peer_node = conf.nodes[idx];
        if (connect_to_proxy(peer_node) == -1) {
            ERROR("failed to connect to QP on %s:%d", peer_node.addr,
                  peer_node.port);
            config_free(&conf);
            close_ssl_ctx(client_ctx);
            return -1;
        }

        INFO("connected to QP on %s:%d", peer_node.addr, peer_node.port);
    }

    config_free(&conf);
    results_map.reserve(concurrent_hint);
    return 0;
}

int close(bool close_remote) {
    // this will close the remote QP
    if (close_remote) {
        int64_t const ticket = gen_ticket(call_type::SYNC);
        flatbuffers::FlatBufferBuilder builder;
        auto close_args = paxos_sgx::crash::CreateEmpty(builder);
        auto request = paxos_sgx::crash::CreateMessage(
            builder, paxos_sgx::crash::MessageType_close_req, ticket,
            paxos_sgx::crash::BasicMessage_Empty, close_args.Union());
        builder.Finish(request);

        size_t const size = builder.GetSize();
        uint8_t const *payload = builder.GetBufferPointer();

        for (auto &server : g_servers) {
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
            process_peer(server, nullptr);  // block, gets released by EOF
        }
    }

    for (auto &server : g_servers) {
        server.close();
    }

    close_ssl_ctx(client_ctx);
    g_servers.clear();
    return 0;
}

size_t n_calls_issued() { return calls_issued; }
size_t n_calls_concluded() { return calls_concluded; }
size_t n_calls_outlasting() { return n_calls_issued() - n_calls_concluded(); }

// sync api
bool fast_get(int64_t account, int64_t &amount) {
    g_fast_get_sync_ctx = FastGetContext();
    int64_t ticket = send_fast_get_request(account, call_type::SYNC);
    fast_get_sync_ticket = ticket;

    if (ticket == -1) {
        ERROR("Failed to send fast get");
        return false;
    }

    while (fast_get_sync_ticket == ticket) {
        struct timeval timeout = global_timeout;
        auto res = process_peers(&timeout);
        if (res == process_res::ERR) {
            return false;
        }
    }

    amount = fast_get_amount_result;
    return fast_get_success_result;
}

bool get(int64_t account, int64_t &amount) {
    return transfer(account, account, 0, amount);
}

bool transfer(int64_t account, int64_t to, int64_t amount,
              int64_t &final_amount) {
    transfer_sync_ticket = send_transfer_request(g_servers[0], account, to,
                                                 amount, call_type::SYNC);
    if (transfer_sync_ticket == -1) {
        ERROR("Failed to transfer");
        return false;
    }
    while (transfer_sync_ticket != -1) {
        if (block_until_return(g_servers[0]) == -1) {
            ERROR("failed to get a return from the basicQP");
            return false;
        }
    }

    final_amount = transfer_amount_result;
    return transfer_success_result;
}

void ping() {
    if (send_ping_request(g_servers[0], call_type::SYNC) == -1) {
        ERROR("Failed to ping");
        return;
    }

    if (block_until_return(g_servers[0]) == -1) {
        ERROR("failed to get a return from the basicQP");
    }
}

void reset() {
    for (auto &server : g_servers) {
        if (send_reset_request(server, call_type::SYNC) == -1) {
            ERROR("Failed to reset");
            return;
        }
        if (block_until_return(server) == -1) {
            ERROR("failed to get a return from the basicQP");
        }
    }
}

// async api
int64_t fast_get_async(int64_t account) {
    int64_t const ticket = send_fast_get_request(account, call_type::ASYNC);
    g_fast_get_ctx_map.emplace(ticket, FastGetContext());
    return ticket;
}
int64_t get_async(int64_t account) {
    return send_transfer_request(g_servers[0], account, account, 0,
                                 call_type::ASYNC);
}
int64_t transfer_async(int64_t account, int64_t to, int64_t amount) {
    return send_transfer_request(g_servers[0], account, to, amount,
                                 call_type::ASYNC);
}
int64_t ping_async() {
    return send_ping_request(g_servers[0], call_type::ASYNC);
}
int64_t reset_async() {
    return send_reset_request(g_servers[0], call_type::ASYNC);
}

// callback api
int fast_get_set_cb(std::function<void(int64_t, int64_t, bool)> cb) {
    fast_get_callback = cb;
    return 0;
}
int64_t fast_get_cb(int64_t account) {
    int64_t const ticket = send_fast_get_request(account, call_type::CALLBACK);
    g_fast_get_ctx_map.emplace(ticket, FastGetContext());
    return ticket;
}

int64_t get_cb(int64_t account) {
    return send_transfer_request(g_servers[0], account, account, 0,
                                 call_type::CALLBACK);
}
int transfer_set_cb(std::function<void(int64_t, int64_t, bool)> cb) {
    transfer_callback = cb;
    return 0;
}
int64_t transfer_cb(int64_t account, int64_t to, int64_t amount) {
    return send_transfer_request(g_servers[0], account, to, amount,
                                 call_type::CALLBACK);
}

int ping_set_cb(std::function<void(int64_t)> cb) {
    ping_callback = cb;
    return 0;
}
int64_t ping_cb() {
    return send_ping_request(g_servers[0], call_type::CALLBACK);
}

int reset_set_cb(std::function<void(int64_t)> cb) {
    reset_callback = cb;
    return 0;
}
int64_t reset_cb() {
    return send_reset_request(g_servers[0], call_type::CALLBACK);
}

// functions to advance state
poll_state poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) return poll_state::READY;
    if (n_calls_outlasting() == 0) return poll_state::NO_CALLS;

    bool is_fast_get =
        (g_fast_get_ctx_map.find(ticket) != std::end(g_fast_get_ctx_map)) ||
        (ticket == -1 && g_fast_get_ctx_map.size() > 0);

    if (!g_servers[0].connected()) {
        ERROR("no connection to proxy");
        return poll_state::ERR;
    }
    if (is_fast_get &&
        std::any_of(std::cbegin(g_servers), std::cend(g_servers),
                    [](peer const &server) { return !server.connected(); })) {
        ERROR("no connection to proxies");
        return poll_state::ERR;
    }

    struct timeval timeout = global_timeout;
    auto const res = (is_fast_get ? process_peers(&timeout)
                                  : process_peer(g_servers[0], &timeout));
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
    auto it = results_map.find(ticket);
    if (it == std::end(results_map)) {
        ERROR("failed to find reply for %ld", ticket);
        return T();
    }
    std::unique_ptr<result> res = std::move(it->second);  // mv constructor
    results_map.erase(it);

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

// for fast, get, transfer
template std::pair<int64_t, bool> get_reply(int64_t ticket);

// for ping reset
template <>
void get_reply(int64_t ticket) {
    auto it = results_map.find(ticket);
    if (it == std::end(results_map)) {
        ERROR("failed to find reply for %ld", ticket);
        return;
    }

    std::unique_ptr<result> res = std::move(it->second);
    if (res->type() != result_type::None)
        ERROR(
            "the template specialization of %s for None was called with "
            "another value",
            __func__);
    results_map.erase(it);
}

// eg: if you are adding an RPC like `double avg(std::vector<int64_t> & const
// v)` you should add the following line `template double get_reply(int64_t
// double)`

// =================================
// protocol helpers
// =================================

namespace {
int64_t send_fast_get_request(int64_t account, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto fast_get_args = paxos_sgx::crash::CreateFastGetArgs(builder, account);

    auto request = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_client_fast_get_req, ticket,
        paxos_sgx::crash::BasicMessage_FastGetArgs, fast_get_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    for (auto &server : g_servers) {
        if (server.append(&size, 1) == -1) {
            // encode message header
            ERROR("failed to prepare message to send");
            return -1;
        }
        if (server.append(payload, size) == -1) {  // then the segment itself
            ERROR("failed to prepare message to send");
            return -1;
        }
    }

    calls_issued++;
    return ticket;
}

int64_t send_transfer_request(peer &server, int64_t account, int64_t to,
                              int64_t amount, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto transfer_args =
        paxos_sgx::crash::CreateOperationArgs(builder, account, to, amount);

    auto request = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_client_operation_req, ticket,
        paxos_sgx::crash::BasicMessage_OperationArgs, transfer_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    calls_issued++;
    return ticket;
}

int64_t send_ping_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_args = paxos_sgx::crash::CreateEmpty(builder);

    auto request = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_ping_req, ticket,
        paxos_sgx::crash::BasicMessage_Empty, ping_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    calls_issued++;
    return ticket;
}

int64_t send_reset_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto reset_args = paxos_sgx::crash::CreateEmpty(builder);

    auto request = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_reset_req, ticket,
        paxos_sgx::crash::BasicMessage_Empty, reset_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    calls_issued++;
    return ticket;
}

// fast
int fast_get_handler_sync(int64_t ticket, int64_t amount, int64_t last_applied,
                          int64_t last_accepted) {
    if (ticket != fast_get_sync_ticket) {
        FINE("already finished %ld (%ld)", ticket, fast_get_sync_ticket);
        return 0;
    }
    if (!g_fast_get_sync_ctx.add_call(amount, last_applied, last_accepted)) {
        fast_get_amount_result = -1;
        fast_get_success_result = false;
        fast_get_sync_ticket = -1;
        calls_concluded++;
    } else if (g_fast_get_sync_ctx.ready(quorum_size())) {
        fast_get_amount_result = amount;
        fast_get_success_result = true;
        fast_get_sync_ticket = -1;
        calls_concluded++;
    }
    return 0;
}
int fast_get_handler_async(int64_t ticket, int64_t amount, int64_t last_applied,
                           int64_t last_accepted) {
    auto it = g_fast_get_ctx_map.find(ticket);
    if (it == g_fast_get_ctx_map.end()) {
        return 0;  // call done
    }

    if (!it->second.add_call(amount, last_applied, last_accepted)) {
        results_map.emplace(
            ticket,
            std::unique_ptr<result>(
                std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                    one_val_result<std::pair<int64_t, bool>>(
                        std::make_pair(-1, false)))));
        g_fast_get_ctx_map.erase(it);
        calls_concluded++;
    } else if (it->second.ready(quorum_size())) {
        results_map.emplace(
            ticket,
            std::unique_ptr<result>(
                std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                    one_val_result<std::pair<int64_t, bool>>(
                        std::make_pair(amount, true)))));
        g_fast_get_ctx_map.erase(it);
        calls_concluded++;
    }

    return 0;
}
int fast_get_handler_cb(int64_t ticket, int64_t amount, int64_t last_applied,
                        int64_t last_accepted) {
    auto it = g_fast_get_ctx_map.find(ticket);
    if (it == g_fast_get_ctx_map.end()) {
        return 0;  // call done
    }

    if (!it->second.add_call(amount, last_applied, last_accepted)) {
        fast_get_callback(ticket, -1, false);
        g_fast_get_ctx_map.erase(it);
        calls_concluded++;
    } else if (it->second.ready(quorum_size())) {
        fast_get_callback(ticket, amount, true);
        g_fast_get_ctx_map.erase(it);
        calls_concluded++;
    }

    return 0;
}
int fast_get_handler(int64_t ticket, int64_t amount, int64_t last_applied,
                     int64_t last_accepted) {
    switch (ticket % 3) {
        case 0:  // SYNC
            return fast_get_handler_sync(ticket, amount, last_applied,
                                         last_accepted);
        case 1:  // ASYNC
            return fast_get_handler_async(ticket, amount, last_applied,
                                          last_accepted);
        case 2:  // CALLBACK
            fast_get_handler_cb(ticket, amount, last_applied, last_accepted);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// operation
int transfer_handler_sync(int64_t amount, bool success) {
    transfer_amount_result = amount;
    transfer_success_result = success;
    transfer_sync_ticket = -1;
    return 0;
}
int transfer_handler_async(int64_t ticket, int64_t amount, bool success) {
    results_map.emplace(
        ticket, std::unique_ptr<result>(
                    std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                        one_val_result<std::pair<int64_t, bool>>(
                            std::make_pair(amount, success)))));
    return 0;
}
int transfer_handler(int64_t ticket, int64_t amount, bool success) {
    switch (ticket % 3) {
        case 0:  // SYNC
            return transfer_handler_sync(amount, success);
        case 1:  // ASYNC
            return transfer_handler_async(ticket, amount, success);
        case 2:  // CALLBACK
            transfer_callback(ticket, amount, success);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// ping
int ping_handler_sync() { return 0; }
int ping_handler_async(int64_t ticket) {
    results_map.emplace(ticket, std::make_unique<result>());
    return 0;
}
int ping_handler(int64_t ticket) {
    switch (ticket % 3) {
        case 0:  // SYNC
            return ping_handler_sync();
        case 1:  // ASYNC
            return ping_handler_async(ticket);
        case 2:  // CALLBACK
            ping_callback(ticket);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// reset
int reset_handler_sync() { return 0; }
int reset_handler_async(int64_t ticket) {
    results_map.emplace(ticket, std::make_unique<result>());
    return 0;
}
int reset_handler(int64_t ticket) {
    switch (ticket % 3) {
        case 0:  // SYNC
            return reset_handler_sync();
        case 1:  // ASYNC
            return reset_handler_async(ticket);
        case 2:  // CALLBACK
            reset_callback(ticket);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

bool has_result(int64_t ticket) {
    return results_map.find(ticket) != std::end(results_map);
}

// =================================
// network helpers
// =================================

int handle_received_message(peer &p) {
    FINE("received a message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("resp size: %zu B (according to header)", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) return 0;

        auto response =
            paxos_sgx::crash::GetMessage(p.buffer().data() + sizeof(size_t));

        switch (response->type()) {
            case paxos_sgx::crash::MessageType_client_fast_get_resp:
                FINE("fast response [ticket %ld]", response->ticket());
                fast_get_handler(
                    response->ticket(),
                    response->message_as_FastGetResult()->amount(),
                    response->message_as_FastGetResult()->last_applied(),
                    response->message_as_FastGetResult()->last_accepted());
                break;
            case paxos_sgx::crash::MessageType_client_operation_resp:
                calls_concluded++;
                FINE("operation response [ticket %ld]", response->ticket());
                transfer_handler(
                    response->ticket(),
                    response->message_as_OperationResult()->amount(),
                    response->message_as_OperationResult()->success());
                break;
            case paxos_sgx::crash::MessageType_ping_resp:
                calls_concluded++;
                FINE("ping response [ticket %ld]", response->ticket());
                ping_handler(response->ticket());
                break;
            case paxos_sgx::crash::MessageType_reset_resp:
                calls_concluded++;
                FINE("reset response [ticket %ld]", response->ticket());
                reset_handler(response->ticket());
                break;
            default:
                calls_concluded++;
                ERROR("Unknown request %d [ticket %ld]", response->type(),
                      response->ticket());
                break;
        }
        p.skip(sizeof(size_t) + total_size);
    }
    return 0;
}

int build_fd_sets(peer const &server, fd_set *read_fds, fd_set *write_fds,
                  fd_set *except_fds) {
    FD_ZERO(read_fds);
    FD_SET(STDIN_FILENO, read_fds);
    FD_SET(server.sock(), read_fds);

    FD_ZERO(write_fds);
    // there is smth to send, set up write_fd for server socket
    if (server.want_write()) FD_SET(server.sock(), write_fds);

    FD_ZERO(except_fds);
    FD_SET(STDIN_FILENO, except_fds);
    FD_SET(server.sock(), except_fds);

    return server.sock();
}

int build_all_fd_sets(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds) {
    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    FD_ZERO(except_fds);
    FD_SET(STDIN_FILENO, read_fds);
    FD_SET(STDIN_FILENO, except_fds);

    int max_sock = 0;
    for (auto const &server : g_servers) {
        FD_SET(server.sock(), read_fds);

        // there is smth to send, set up write_fd for server socket
        if (server.want_write()) {
            FD_SET(server.sock(), write_fds);
        }

        FD_SET(server.sock(), except_fds);
        if (server.sock() > max_sock) {
            max_sock = server.sock();
        }
    }

    return max_sock;
}

int block_until_return(peer &server) {
    /* event loop */
    while (1) {
        struct timeval timeout = global_timeout;
        auto res = process_peer(server, &timeout);
        if (res == process_res::ERR)
            return -1;
        else if (res == process_res::HANDLED_MSG)
            return 0;
    }
}

int connect_to_proxy(config_node_t const &peer_node) {
    g_servers.emplace_back(client_ctx, false);
    peer &server = g_servers[g_servers.size() - 1];
    FINE("connecting to peer in %s:%d", peer_node.addr, peer_node.port);
    if (server.connect(peer_node.addr, peer_node.port) != 0) {
        server.close();
        ERROR("failed to connect to peer");
        return -1;
    }

    while (!server.finished_handshake()) {
        if (server.handshake() != 0) {
            server.close();
            ERROR("failed to do handshake");
            return -1;
        }
        if (server.want_write()) {
            if (server.send() != 0) {
                server.close();
                return -1;
            }
        }
        if (server.recv() != 0) {
            ERROR("failed to recv on hanshake");
            server.close();
            return -1;
        }
    }

    while (server.connected() && !server.finished_handshake()) {
        struct timeval timeout = global_timeout;
        if (process_peer(server, &timeout) == process_res::ERR) return -1;
    }

    if (!server.connected()) {
        ERROR("failed to finish handshake");
        server.close();
        return -1;
    }

    FINE("connected to peer in %s:%d", peer_node.addr, peer_node.port);

    return 0;
}

process_res process_peer(peer &p, struct timeval *timeout) {
    if (p.connected() && p.want_read()) {
        handle_received_message(p);
        return process_res::HANDLED_MSG;
    }
    if (p.want_flush()) p.flush();

    fd_set read_fds, write_fds, except_fds;
    int high_sock = build_fd_sets(p, &read_fds, &write_fds, &except_fds);
    int activity =
        select(high_sock + 1, &read_fds, &write_fds, &except_fds, timeout);

    switch (activity) {
        case -1:
            ERROR("failed to select: %s", strerror(errno));
            p.close();
            return process_res::ERR;

        case 0:
            break;

        default:
            if (p.connected()) {
                if (FD_ISSET(p.sock(), &read_fds)) {
                    if (p.recv() != 0) {
                        ERROR("failed to receive from server");
                        p.close();
                        return process_res::ERR;
                    }
                }
                if (FD_ISSET(p.sock(), &write_fds)) {
                    if (p.send() != 0) {
                        ERROR("failed to send to server");
                        p.close();
                        return process_res::ERR;
                    }
                }
                if (FD_ISSET(p.sock(), &except_fds)) {
                    ERROR("exception on server socket");
                    p.close();
                    return process_res::ERR;
                }
            }
    }

    return process_res::NOOP;
}

process_res process_peers(struct timeval *timeout) {
    for (auto &server : g_servers) {
        if (server.connected() && server.want_read()) {
            handle_received_message(server);
        }
    }

    for (auto &server : g_servers) {
        if (server.want_flush()) {
            server.flush();
        }
    }

    fd_set read_fds, write_fds, except_fds;
    int high_sock = build_all_fd_sets(&read_fds, &write_fds, &except_fds);
    int activity =
        select(high_sock + 1, &read_fds, &write_fds, &except_fds, timeout);

    switch (activity) {
        case -1:
            ERROR("failed to select: %s", strerror(errno));
            return process_res::ERR;

        case 0:
            break;

        default:
            for (auto &server : g_servers) {
                if (server.connected()) {
                    if (FD_ISSET(server.sock(), &read_fds)) {
                        if (server.recv() != 0) {
                            ERROR("failed to receive from server");
                            server.close();
                            return process_res::ERR;
                        }
                    }
                    if (FD_ISSET(server.sock(), &write_fds)) {
                        if (server.send() != 0) {
                            ERROR("failed to send to server");
                            server.close();
                            return process_res::ERR;
                        }
                    }
                    if (FD_ISSET(server.sock(), &except_fds)) {
                        ERROR("exception on server socket");
                        server.close();
                        return process_res::ERR;
                    }
                }
            }
    }

    return process_res::NOOP;
}

}  // anonymous namespace

}  // namespace crash
}  // namespace paxos_sgx
