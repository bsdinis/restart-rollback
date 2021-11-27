/**
 * enclave.c
 *
 * implement enclave stuff
 */

#include "enclave_t.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "fd_helper.h"
#include "handlers.h"
#include "setup.h"

#include <vector>
#include "log.h"
#include "peer.h"
#include "perf.h"

namespace paxos_sgx {
namespace crash {
perf::perf_recorder perf_rec;
}  // namespace crash
}  // namespace paxos_sgx

namespace {
// client list
std::vector<peer> client_list;

int process_peer(peer &p);
}  // anonymous namespace

using namespace paxos_sgx::crash;

void crash_enclave_start(config_t conf, ssize_t my_idx, void *measure_buffer,
                           size_t buffer_size) {
    perf_rec = perf::perf_recorder(measure_buffer, buffer_size);
    setup::setup(&conf, my_idx);
    INFO("setup on %s:%d", conf.nodes[my_idx].addr, conf.nodes[my_idx].port);
    int const listen_sock = setup::listen_sock();

    while (!setup::closed()) {
        for (auto &peer : client_list) {
            if (peer.connected() && peer.want_read()) {
                handler::handle_client_message(peer);
                if (setup::closed()) return;
            }

            if (peer.want_flush()) peer.flush();
        }

        int activity = net::select_list(listen_sock, client_list);

        int rd;
        int err;
        switch (activity) {
            case -1:
                KILL("select returned -1");
                return;

            case 0:
                FINE("timeout");
                break;

            default:
                ocall_needs_read(&rd, listen_sock);
                ocall_needs_except(&err, listen_sock);
                if (rd) {
                    if (handler::handle_new_connection(listen_sock,
                                                       client_list) != 0) {
                        ERROR("Failed to handle new client connection");
                    }
                }
                if (err) {
                    ERROR("Exception on listening socket %d", listen_sock);
                    return;
                }

                std::for_each(begin(client_list), end(client_list),
                              process_peer);
        }
    }
}

namespace {
int process_peer(peer &p) {
    if (p.connected()) {
        int rd;
        int wr;
        int err;
        ocall_needs_read(&rd, p.sock());
        ocall_needs_write(&wr, p.sock());
        ocall_needs_except(&err, p.sock());
        if (rd) {
            errno = 0;
            if (p.recv() != 0) {
                ERROR("failed to recv from peer");
                p.close();
                INFO("closed connection with peer");
                return -1;
            }
        }
        if (p.connected() && wr) {
            errno = 0;
            if (p.send() != 0) {
                ERROR("failed to send to peer");
                p.close();
                INFO("closed connection with peer");
                return -1;
            }
        }
        if (p.connected() && err) {
            errno = 0;
            ERROR("exception on peer");
            p.close();
            INFO("closed connection with peer: %s (%d)", strerror(errno),
                 errno);
            return -1;
        }
    }

    return 0;
}
}  // anonymous namespace
