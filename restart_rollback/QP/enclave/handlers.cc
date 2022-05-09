/**
 * handlers.c
 *
 * handlers for the assorted versions of problems
 */

#include "handlers.h"

#include "log.h"

#include "peer.h"
#include "perf.h"
#include "setup.h"
#include "timer.h"

#include "restart_rollback_generated.h"

// XXX: CHANGE ME
// add other operation handlers
#include "close_handler.h"
#include "handler_helpers.h"
#include "ping_handler.h"
#include "protocol_handler.h"
#include "proxy_handler.h"
#include "reset_handler.h"

#include <string.h>
#include <algorithm>
#include <iterator>

using namespace register_sgx::restart_rollback;

namespace register_sgx {
namespace restart_rollback {
extern perf::perf_recorder perf_rec;
}  // namespace restart_rollback
}  // namespace register_sgx

namespace {
int peer_new_connection(int const listen_socket, std::vector<peer> &list,
                        ssize_t idx = -1);
size_t constexpr connection_limit = 1 << 13;
int32_t g_current_id = 0;
}  // anonymous namespace

namespace register_sgx {
namespace restart_rollback {
namespace handler {

ssize_t handle_new_connection(int const listen_socket,
                              std::vector<peer> &list) {
    LOG("accepting new connection on socket %d", listen_socket);
    auto it = std::find_if_not(std::begin(list), std::end(list),
                               [](peer &p) -> bool { return p.connected(); });
    ssize_t idx = std::distance(std::begin(list), it);
    LOG("index %zd", idx);
    if (it == std::end(list)) {
        if (peer_new_connection(listen_socket, list) != 0) {
            return -1;
        }
    } else if (peer_new_connection(listen_socket, list, idx) != 0) {
        return -1;
    }

    return idx;
}

int handle_client_message(peer &p) {
    FINE("recvd a client message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("request size: %zu B", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) return 0;

        auto request = register_sgx::restart_rollback::GetMessage(
            p.buffer().data() + sizeof(size_t));

        auto const begin = timer::now();
        switch (request->type()) {
            case register_sgx::restart_rollback::MessageType_get_req:
                get_handler(p, request->ticket(),
                            request->message_as_GetArgs());
                perf_rec.add("get", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_get_timestamp_req:
                get_timestamp_handler(p, request->ticket(),
                                      request->message_as_GetTimestampArgs());
                perf_rec.add("get_timestamp", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_put_req:
                put_handler(p, request->ticket(),
                            request->message_as_PutArgs());
                perf_rec.add("put", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_proxy_get_req:
                proxy_get_handler(p, request->ticket(),
                                  request->message_as_GetArgs());
                perf_rec.add("get", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_proxy_put_req:
                proxy_put_handler(p, request->ticket(),
                                  request->message_as_ProxyPutArgs());
                perf_rec.add("put", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_ping_req:
                client_ping_handler(p, request->ticket());
                perf_rec.add("ping");
                break;
            case register_sgx::restart_rollback::MessageType_reset_req:
                reset_handler(p, request->ticket());
                perf_rec.add("reset");
                break;
            case register_sgx::restart_rollback::MessageType_close_req:
                close_handler(p, request->ticket());
                break;
            default:
                // XXX: CHANGE ME
                // add other operation switch
                // (don't forget the intrusive perf)
                ERROR("Unknown client request %d [ticket %ld]", request->type(),
                      request->ticket());
        }
        p.skip(sizeof(size_t) + total_size);
    }

    return 0;
}

int handle_replica_message(peer &p, size_t idx) {
    FINE("recvd a replica message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("incoming size: %zu B", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) {
            return 0;
        }

        auto message = register_sgx::restart_rollback::GetMessage(
            p.buffer().data() + sizeof(size_t));
        auto const begin = timer::now();
        switch (message->type()) {
            case register_sgx::restart_rollback::MessageType_get_req:
                get_handler(p, message->ticket(),
                            message->message_as_GetArgs());
                perf_rec.add("get", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_get_resp:
                get_resp_handler(p, idx, message->ticket(),
                                 message->message_as_GetResult());
                perf_rec.add("get", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_get_timestamp_req:
                get_timestamp_handler(p, message->ticket(),
                                      message->message_as_GetTimestampArgs());
                perf_rec.add("get_timestamp", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_get_timestamp_resp:
                get_timestamp_resp_handler(
                    p, message->ticket(),
                    message->message_as_GetTimestampResult());
                perf_rec.add("get timestamp", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_put_req:
                put_handler(p, message->ticket(),
                            message->message_as_PutArgs());
                perf_rec.add("put", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_put_resp:
                put_resp_handler(p, message->ticket(),
                                 message->message_as_PutResult());
                perf_rec.add("put", timer::elapsed_usec(begin));
                break;
            case register_sgx::restart_rollback::MessageType_ping_req:
                replica_ping_handler(p, message->ticket());
                perf_rec.add("ping");
                break;
            case register_sgx::restart_rollback::MessageType_ping_resp:
                perf_rec.add("ping");
                break;
            case register_sgx::restart_rollback::MessageType_reset_req:
                reset_handler(p, message->ticket());
                perf_rec.add("reset");
                break;
            default:
                // XXX: CHANGE ME
                // add other operation switch
                // (don't forget the intrusive perf)
                ERROR("Unknown replica message %d [ticket %ld]",
                      message->type(), message->ticket());
                p.skip(sizeof(size_t) + total_size);
                return -1;
        }

        p.skip(sizeof(size_t) + total_size);
    }

    return 0;
}

int send_greeting(peer &p) {
    LOG("sending greeting to client %d", g_current_id);
    flatbuffers::FlatBufferBuilder builder;

    auto greeting =
        register_sgx::restart_rollback::CreateGreeting(builder, g_current_id);
    g_current_id += 1;

    auto message = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_client_greeting,
        -1, register_sgx::restart_rollback::BasicMessage_Greeting,
        greeting.Union());
    builder.Finish(message);

    return register_sgx::restart_rollback::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx

namespace {

int peer_new_connection(int const listen_socket, std::vector<peer> &list,
                        ssize_t idx) {
    if (idx == -1) {
        if (list.size() >= connection_limit) {
            ERROR("this QP has a limit of %zu connections, ignoring new one",
                  connection_limit);
            return -1;
        }

        try {
            LOG("adding to the end");
            list.emplace_back(register_sgx::restart_rollback::setup::ssl_ctx(),
                              true);
        } catch (std::bad_alloc &) {
            ERROR("allocation failure: cannot add a new peer to list");
            return -1;
        }
    } else {
        LOG("adding to the %zd", idx);
        list.emplace(std::begin(list) + idx,
                     register_sgx::restart_rollback::setup::ssl_ctx(), true);
    }
    peer &p = idx == -1 ? *(std::end(list) - 1) : list[idx];

    if (p.accept(listen_socket) != 0) {
        ERROR("failed to accept connection");
        return -1;
    }

    while (!p.finished_handshake()) {
        if (p.handshake() == -1) {
            ERROR("failed to do handshake");
            return -1;
        }
        if (p.want_write()) {
            if (p.send() == -1) {
                ERROR("failed to send on hanshake");
                p.close();
                return -1;
            }
        }
        if (p.recv(true /* ignoreEOF */) == -1) {
            ERROR("failed to recv on hanshake");
            p.close();
            return -1;
        }
    }

    LOG("accepted new connection on socket %d", listen_socket);
    return 0;
}

}  // namespace
