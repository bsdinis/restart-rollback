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

#include "basicQP_generated.h"
#include "basicQP_req_generated.h"

// XXX: CHANGE ME
// add other operation handlers
#include "close_handler.h"
#include "ping_handler.h"
#include "sum_handler.h"

#include <string.h>
#include <algorithm>
#include <iterator>

using namespace epidemics::basicQP;

namespace epidemics {
namespace basicQP {
extern perf::perf_recorder perf_rec;
}  // namespace basicQP
}  // namespace epidemics

namespace {
int peer_new_connection(int const listen_socket, std::vector<peer> &list,
                        ssize_t idx = -1);
size_t constexpr connection_limit = 1 << 13;
}  // anonymous namespace

namespace epidemics {
namespace basicQP {
namespace handler {

int handle_new_connection(int const listen_socket, std::vector<peer> &list) {
    LOG("accepting new connection on socket %d", listen_socket);
    auto it = std::find_if_not(std::begin(list), std::end(list),
                               [](peer &p) -> bool { return p.connected(); });
    return it == std::end(list)
               ? peer_new_connection(listen_socket, list)
               : peer_new_connection(listen_socket, list,
                                     std::distance(std::begin(list), it));
}

int handle_client_message(peer &p) {
    FINE("recvd a message: %zu B", p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("request size: %zu B", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) return 0;

        auto request = epidemics::basicQP::GetBasicRequest(p.buffer().data() +
                                                           sizeof(size_t));

        auto const begin = timer::now();
        switch (request->type()) {
            case epidemics::basicQP::ReqType_sum:
                sum_handler(p, request->ticket(),
                            *request->args_as_SumArgs()->vec());
                perf_rec.add("sum", timer::elapsed_usec(begin));
                break;
            case epidemics::basicQP::ReqType_ping:
                ping_handler(p, request->ticket());
                perf_rec.add("ping");
                break;
            case epidemics::basicQP::ReqType_close:
                close_handler(p, request->ticket());
                break;
            default:
                // XXX: CHANGE ME
                // add other operation switch
                // (don't forget the intrusive perf)
                ERROR("Unknown request %d [ticket %ld]", request->type(),
                      request->ticket());
        }
        p.skip(sizeof(size_t) + total_size);
    }

    return 0;
}

}  // namespace handler
}  // namespace basicQP
}  // namespace epidemics

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
            list.emplace_back(epidemics::basicQP::setup::ssl_ctx(), true);
        } catch (std::bad_alloc &) {
            ERROR("allocation failure: cannot add a new peer to list");
            return -1;
        }
    } else {
        list.emplace(std::begin(list) + idx,
                     epidemics::basicQP::setup::ssl_ctx(), true);
    }
    peer &p = idx == -1 ? *(std::end(list) - 1) : list[idx];

    if (p.accept(listen_socket) != 0) {
        ERROR("failed to accept connection");
        return -1;
    }

    while (!p.finished_handshake()) {
        if (p.handshake() != 0) {
            ERROR("failed to do handshake");
            return -1;
        }
        if (p.want_write()) {
            if (p.send() != 0) {
                ERROR("failed to send on hanshake");
                p.close();
                return -1;
            }
        }
        if (p.recv() != 0) {
            ERROR("failed to recv on hanshake");
            p.close();
            return -1;
        }
    }

    LOG("accepted new connection on socket %d", listen_socket);
    return 0;
}

}  // namespace
