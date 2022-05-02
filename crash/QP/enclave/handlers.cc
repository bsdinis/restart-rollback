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

#include "crash_generated.h"

// XXX: CHANGE ME
// add other operation handlers
#include "client_handler.h"
#include "close_handler.h"
#include "ping_handler.h"
#include "replica_handler.h"
#include "reset_handler.h"

#include <string.h>
#include <algorithm>
#include <iterator>

using namespace register_sgx::crash;

namespace register_sgx {
namespace crash {
extern perf::perf_recorder perf_rec;
}  // namespace crash
}  // namespace register_sgx

namespace {
int peer_new_connection(int const listen_socket, std::vector<peer> &list,
                        ssize_t idx = -1);
size_t constexpr connection_limit = 1 << 13;
}  // anonymous namespace

namespace register_sgx {
namespace crash {
namespace handler {

int handle_new_connection(int const listen_socket, std::vector<peer> &list) {
    LOG("accepting new connection on socket %d", listen_socket);
    auto it = std::find_if_not(std::begin(list), std::end(list),
                               [](peer &p) -> bool { return p.connected(); });
    LOG("index %zu", std::distance(std::begin(list), it));
    return it == std::end(list)
               ? peer_new_connection(listen_socket, list)
               : peer_new_connection(listen_socket, list,
                                     std::distance(std::begin(list), it));
}

int handle_client_message(peer &p) {
    FINE("recvd a client message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("request size: %zu B", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) return 0;

        auto request =
            register_sgx::crash::GetMessage(p.buffer().data() + sizeof(size_t));

        auto const begin = timer::now();
        switch (request->type()) {
            case register_sgx::crash::MessageType_get_req:
                client_get_handler(p, request->ticket(),
                                   request->message_as_GetArgs());
                perf_rec.add("get", timer::elapsed_usec(begin));
                break;
            case register_sgx::crash::MessageType_get_timestamp_req:
                client_get_timestamp_handler(
                    p, request->ticket(),
                    request->message_as_GetTimestampArgs());
                perf_rec.add("get_timestamp", timer::elapsed_usec(begin));
                break;
            case register_sgx::crash::MessageType_put_req:
                client_put_handler(p, request->ticket(),
                                   request->message_as_PutArgs());
                perf_rec.add("put", timer::elapsed_usec(begin));
                break;
            case register_sgx::crash::MessageType_ping_req:
                client_ping_handler(p, request->ticket());
                perf_rec.add("ping");
                break;
            case register_sgx::crash::MessageType_reset_req:
                reset_handler(p, request->ticket());
                perf_rec.add("reset");
                break;
            case register_sgx::crash::MessageType_close_req:
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

int handle_replica_message(peer &p) {
    FINE("recvd a replica message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("incoming size: %zu B", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) {
            return 0;
        }

        auto message =
            register_sgx::crash::GetMessage(p.buffer().data() + sizeof(size_t));
        auto const begin = timer::now();
        switch (message->type()) {
            case register_sgx::crash::MessageType_ping_req:
                replica_ping_handler(p, message->ticket());
                perf_rec.add("ping");
                break;
            case register_sgx::crash::MessageType_ping_resp:
                INFO("ping response [%ld]", message->ticket());
                perf_rec.add("ping");
                break;
            case register_sgx::crash::MessageType_reset_req:
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

}  // namespace handler
}  // namespace crash
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
            list.emplace_back(register_sgx::crash::setup::ssl_ctx(), true);
        } catch (std::bad_alloc &) {
            ERROR("allocation failure: cannot add a new peer to list");
            return -1;
        }
    } else {
        LOG("adding to the %zd", idx);
        list.emplace(std::begin(list) + idx,
                     register_sgx::crash::setup::ssl_ctx(), true);
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
