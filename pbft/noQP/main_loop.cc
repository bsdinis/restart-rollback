/**
 * main_loop.c
 *
 * implement main interaction loop
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "config.h"
#include "fd_helper.h"
#include "handlers.h"
#include "setup.h"

#include <vector>
#include "log.h"
#include "peer.h"
#include "perf.h"

namespace {
// client list
std::vector<peer> client_list;

int process_peer(peer &p);

fd_set read_fds;
fd_set write_fds;
fd_set except_fds;

}  // anonymous namespace

namespace paxos_sgx {
namespace pbft {

perf::perf_recorder perf_rec;

void basicQP_enclave_start(config_t conf, ssize_t my_idx, void *measure_buffer,
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

        int activity = net::select_list(listen_sock, client_list, &read_fds,
                                        &write_fds, &except_fds);

        switch (activity) {
            case -1:
                KILL("select returned -1");
                return;

            case 0:
                FINE("timeout");
                break;

            default:
                if (FD_ISSET(listen_sock, &read_fds)) {
                    if (handler::handle_new_connection(listen_sock,
                                                       client_list) != 0) {
                        ERROR("Failed to handle new client connection");
                    }
                }
                if (FD_ISSET(listen_sock, &except_fds)) {
                    ERROR("Exception on listening socket %d", listen_sock);
                    return;
                }

                std::for_each(begin(client_list), end(client_list),
                              process_peer);
        }
    }
}

}  // namespace pbft
}  // namespace paxos_sgx

namespace {
int process_peer(peer &p) {
    if (p.connected()) {
        if (FD_ISSET(p.sock(), &read_fds)) {
            errno = 0;
            if (p.recv() != 0) {
                ERROR("failed to recv from peer");
                p.close();
                INFO("closed connection with peer");
                return -1;
            }
        }
        if (p.connected() && FD_ISSET(p.sock(), &write_fds)) {
            errno = 0;
            if (p.send() != 0) {
                ERROR("failed to send to peer");
                p.close();
                INFO("closed connection with peer");
                return -1;
            }
        }
        if (p.connected() && FD_ISSET(p.sock(), &except_fds)) {
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
