/**
 * qp_clnt.c
 */

#include "qp_clnt.h"
#include "log.h"
#include "qp_result.h"

#include "config.h"

#include "peer.h"
#include "ssl_util.h"

#include "basicQP_generated.h"
#include "basicQP_req_generated.h"
#include "basicQP_resp_generated.h"

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

namespace epidemics {
namespace basicQP {

namespace {
// protocol helpers
enum class call_type { SYNC, ASYNC, CALLBACK };
// sum
int64_t send_sum_request(peer &server, std::vector<int64_t> const &vec,
                         call_type type);
int sum_handler(int64_t ticket, int64_t result);
int sum_handler_sync(int64_t sum);
int sum_handler_async(int64_t ticket, int64_t sum);
std::function<void(int64_t, int64_t)> sum_callback = [](int64_t, int64_t) {};
// ping
int64_t send_ping_request(peer &server, call_type type);
int ping_handler(int64_t ticket);
int ping_handler_sync();
int ping_handler_async(int64_t ticket);
std::function<void(int64_t)> ping_callback = [](int64_t) {};
// XXX: CHANGE ME
// add new protocol helpers

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

// global variables to channel the results of synchronous calls to
int64_t sum_result;
// XXX: CHANGE ME
// add results (when required)

// async results
std::unordered_map<int64_t, std::unique_ptr<result>> results_map;
bool has_result(int64_t ticket);

// network globals
SSL_CTX *client_ctx = nullptr;
peer *server = nullptr;
timeval global_timeout;

// network helpers
enum class process_res { HANDLED_MSG, NOOP, ERR };
int handle_received_message(peer &p);
int build_fd_sets(peer const &server, fd_set *read_fds, fd_set *write_fds,
                  fd_set *except_fds);
int block_until_return(peer &server);
int connect_to_proxy(config_node_t const &peer_node);
process_res process_peer(peer &p, struct timeval *timeout = nullptr);
}  // namespace

// =================================
// public implementation
// =================================
int init(
        char const * config,     // config file
        ssize_t idx,             // index in the config; TODO: make -1 mean login to all and add idx in other calls
        size_t concurrent_hint,  // hint for number of concurrent calls permitted
        struct timeval timeout,  // timeout for select
        char const * cert_path,  // certificate for the client
        char const * key_path    // private key of the certificate
) {
    global_timeout = timeout;
    config_t conf;
    if (config_parse(&conf, config) == -1) {
        ERROR("failed to stat configuration: %s", config);
        return -1;
    }
    if (idx < 0 || idx >= conf.size) {
        ERROR("index %zd out of bounds (size = %zu)", idx, conf.size);
        return -1;
    }

    if (init_client_ssl_ctx(&client_ctx) == -1) {
        ERROR("failed to setup ssl ctx");
        return -1;
    }

    if (load_certificates(client_ctx, cert_path, key_path) == -1) {
        ERROR("failed load certs");
        return -1;
    }


    auto const &peer_node = conf.nodes[idx];
    if (connect_to_proxy(peer_node) == -1) {
        ERROR("failed to connect to QP on %s:%d", peer_node.addr,
              peer_node.port);
        config_free(&conf);
        close_ssl_ctx(client_ctx);
        return -1;
    }

    INFO("connected to QP on %s:%d", peer_node.addr, peer_node.port);
    config_free(&conf);
    results_map.reserve(concurrent_hint);
    return 0;
}

int close(bool close_remote) {
    // this will close the remote QP
    if (close_remote) {
        int64_t const ticket = gen_ticket(call_type::SYNC);
        flatbuffers::FlatBufferBuilder builder;
        auto close_args = epidemics::basicQP::CreatePingArgs(builder);
        auto request = epidemics::basicQP::CreateBasicRequest(
            builder, epidemics::basicQP::ReqType_close, ticket,
            epidemics::basicQP::ReqArgs_CloseArgs, close_args.Union());
        builder.Finish(request);

        size_t const size = builder.GetSize();
        uint8_t const *payload = builder.GetBufferPointer();

        if (server->append(&size, 1) == -1) {
            // encode message header
            ERROR("failed to prepare message to send");
            return -1;
        }
        if (server->append(payload, size) == -1) {  // then the segment itself
            ERROR("failed to prepare message to send");
            return -1;
        }

        server->flush();
        process_peer(*server, nullptr); // block, gets released by EOF
    }

    close_ssl_ctx(client_ctx);
    delete server;
    server = nullptr;
    return 0;
}

size_t n_calls_issued() { return calls_issued; }
size_t n_calls_concluded() { return calls_concluded; }
size_t n_calls_outlasting() { return n_calls_issued() - n_calls_concluded(); }

// sync api
int64_t sum(std::vector<int64_t> const &vec) {
    if (send_sum_request(*server, vec, call_type::SYNC) == -1) {
        ERROR("Failed to sum");
        return -1;
    }
    if (block_until_return(*server) == -1) {
        ERROR("failed to get a return from the basicQP");
        return -1;
    }

    return sum_result;
}

void ping() {
    if (send_ping_request(*server, call_type::SYNC) == -1) {
        ERROR("Failed to ping");
        return;
    }

    if (block_until_return(*server) == -1) {
        ERROR("failed to get a return from the basicQP");
    }
}

// XXX: CHANGE ME
// implement sync functions

// async api
int64_t sum_async(std::vector<int64_t> const &vec) {
    return send_sum_request(*server, vec, call_type::ASYNC);
}

int64_t ping_async() { return send_ping_request(*server, call_type::ASYNC); }

// XXX: CHANGE ME
// implement async functions

// callback api
int sum_set_cb(std::function<void(int64_t, int64_t)> cb) {
    sum_callback = cb;
    return 0;
}
int64_t sum_cb(std::vector<int64_t> const &vec) {
    return send_sum_request(*server, vec, call_type::CALLBACK);
}

int ping_set_cb(std::function<void(int64_t)> cb) {
    ping_callback = cb;
    return 0;
}
int64_t ping_cb() { return send_ping_request(*server, call_type::CALLBACK); }

// XXX: CHANGE ME
// implement callback functions

// functions to advance state
poll_state poll(int64_t ticket) {
    if (n_calls_outlasting() == 0) return poll_state::NO_CALLS;
    if (ticket != -1 && has_result(ticket)) return poll_state::READY;

    if (!server->connected()) {
        ERROR("no connection to proxy");
        return poll_state::ERR;
    }

    struct timeval timeout = global_timeout;
    auto const res = process_peer(*server, &timeout);
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

// for sum
template int64_t get_reply(int64_t ticket);

// for ping
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

// XXX: CHANGE ME
// add required template instantiations
//
// eg: if you are adding an RPC like `double avg(std::vector<int64_t> & const
// v)` you should add the following line `template double get_reply(int64_t
// double)`

// =================================
// protocol helpers
// =================================

namespace {
int64_t send_sum_request(peer &server, std::vector<int64_t> const &vec,
                         call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto sum_args = epidemics::basicQP::CreateSumArgsDirect(builder, &vec);

    auto request = epidemics::basicQP::CreateBasicRequest(
        builder, epidemics::basicQP::ReqType_sum, ticket,
        epidemics::basicQP::ReqArgs_SumArgs, sum_args.Union());
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
    auto ping_args = epidemics::basicQP::CreatePingArgs(builder);

    auto request = epidemics::basicQP::CreateBasicRequest(
        builder, epidemics::basicQP::ReqType_ping, ticket,
        epidemics::basicQP::ReqArgs_PingArgs, ping_args.Union());
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

// XXX: CHANGE ME
// add send function for new operations

// sum
int sum_handler_sync(int64_t sum) {
    sum_result = sum;
    return 0;
}
int sum_handler_async(int64_t ticket, int64_t sum) {
    results_map.emplace(ticket, std::unique_ptr<result>(
                                    std::make_unique<one_val_result<int64_t>>(
                                        one_val_result<int64_t>(sum))));
    return 0;
}
int sum_handler(int64_t ticket, int64_t sum) {
    switch (ticket % 3) {
        case 0:  // SYNC
            return sum_handler_sync(sum);
        case 1:  // ASYNC
            return sum_handler_async(ticket, sum);
        case 2:  // CALLBACK
            sum_callback(ticket, sum);
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

// XXX: CHANGE ME
// add handlers for new operations

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

        auto response = epidemics::basicQP::GetBasicResponse(p.buffer().data() +
                                                             sizeof(size_t));

        calls_concluded++;
        switch (response->type()) {
            case epidemics::basicQP::ReqType_sum:
                FINE("sum response [ticket %ld]", response->ticket());
                sum_handler(response->ticket(),
                            response->result_as_SumResult()->sum());
                break;
            case epidemics::basicQP::ReqType_ping:
                FINE("ping response [ticket %ld]", response->ticket());
                ping_handler(response->ticket());
                break;
            default:
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

int block_until_return(peer &server) {
    /* event loop */
    while (1) {
        auto res = process_peer(server);
        if (res == process_res::ERR)
            return -1;
        else if (res == process_res::HANDLED_MSG)
            return 0;
    }
}

int connect_to_proxy(config_node_t const &peer_node) {
    server = new peer(client_ctx, false);
    FINE("connecting to peer in %s:%d", peer_node.addr, peer_node.port);
    if (server->connect(peer_node.addr, peer_node.port) != 0) {
        server->close();
        ERROR("failed to connect to peer");
        return -1;
    }

    while (!server->finished_handshake()) {
        if (server->handshake() != 0) {
            server->close();
            ERROR("failed to do handshake");
            return -1;
        }
        if (server->want_write()) {
            if (server->send() != 0) {
                server->close();
                return -1;
            }
        }
        if (server->recv() != 0) {
            ERROR("failed to recv on hanshake");
            server->close();
            return -1;
        }
    }

    while (server->connected() && !server->finished_handshake()) {
        if (process_peer(*server) == process_res::ERR) return -1;
    }

    if (!server->connected()) {
        ERROR("failed to finish handshake");
        server->close();
        return -1;
    }

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

}  // anonymous namespace

}  // namespace basicQP
}  // namespace epidemics
