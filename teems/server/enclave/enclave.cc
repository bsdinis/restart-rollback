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

namespace teems {
perf::perf_recorder perf_rec;
}  // namespace teems

extern std::vector<peer> g_client_list;
extern std::vector<peer> g_replica_list;

namespace {
int process_peer(peer &p);
}  // anonymous namespace

using namespace teems;

void teems_enclave_start(config_t conf, ssize_t my_idx, void *file_mapping,
                         size_t mapping_size, void *measure_buffer,
                         size_t buffer_size, size_t f) {
    perf_rec = perf::perf_recorder(measure_buffer, buffer_size);
    setup::setup(&conf, my_idx, file_mapping, mapping_size, f);
    INFO("setup on %s:%d and %s:%d", conf.nodes[my_idx].addr,
         conf.nodes[my_idx].port, conf.nodes[my_idx].addr,
         conf.nodes[my_idx].port * 2);
    INFO("write quorum size: %zu", setup::write_quorum_size());
    INFO("read quorum size:  %zu + s", setup::read_quorum_size(0));
    int const client_listen_sock = setup::client_listen_sock();
    int const replica_listen_sock = setup::replica_listen_sock();

    while (!setup::closed()) {
        for (auto &peer : g_client_list) {
            if (peer.connected() && peer.want_read()) {
                handler::handle_client_message(peer);
                if (setup::closed()) return;
            }
        }

        for (size_t idx = 0; idx < g_replica_list.size(); ++idx) {
            auto &peer = g_replica_list[idx];
            if (peer.connected() && peer.want_read()) {
                handler::handle_replica_message(peer, idx);
                if (setup::closed()) return;
            }

            if (peer.want_flush()) peer.flush();
        }

        for (auto &peer : g_client_list) {
            if (peer.want_flush()) peer.flush();
        }
        for (auto &peer : g_replica_list) {
            if (peer.want_flush()) peer.flush();
        }

        int activity = net::select_list(client_listen_sock, replica_listen_sock,
                                        g_client_list, g_replica_list);

        int rd = 0;
        int err = 0;
        switch (activity) {
            case -1:
                KILL("select returned -1");
                return;

            case 0:
                FINE("timeout");
                break;

            default:
                ocall_needs_read(&rd, client_listen_sock);
                ocall_needs_except(&err, client_listen_sock);
                if (rd) {
                    ssize_t idx = handler::handle_new_connection(
                        client_listen_sock, g_client_list);
                    if (idx == -1) {
                        ERROR("Failed to handle new client connection");
                    } else if (setup::my_idx() == 0) {
                        teems::handler::send_greeting(g_client_list[idx]);
                    }
                }
                if (err) {
                    ERROR("Exception on listening socket %d",
                          client_listen_sock);
                    return;
                }
                rd = 0;
                err = 0;

                ocall_needs_read(&rd, replica_listen_sock);
                ocall_needs_except(&err, replica_listen_sock);
                if (rd) {
                    if (handler::handle_new_connection(replica_listen_sock,
                                                       g_replica_list) == -1) {
                        ERROR("Failed to handle new replica connection");
                    }
                }
                if (err) {
                    ERROR("Exception on listening socket %d",
                          replica_listen_sock);
                    return;
                }

                std::for_each(begin(g_client_list), end(g_client_list),
                              process_peer);
                std::for_each(begin(g_replica_list), end(g_replica_list),
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
