#include "qp_network.h"
#include "qp_helpers.h"

#include "log.h"
#include "peer.h"
#include "ssl_util.h"

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

namespace register_sgx {
namespace restart_rollback {

extern SSL_CTX *g_client_ctx;
extern timeval g_timeout;
extern ::std::vector<peer> g_servers;

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

int block_until_return(size_t idx, peer &server) {
    /* event loop */
    while (1) {
        struct timeval timeout = g_timeout;
        auto res = process_peer(idx, server, &timeout);
        if (res == process_res::ERR)
            return -1;
        else if (res == process_res::HANDLED_MSG)
            return 0;
    }
}

int connect_to_proxy(config_node_t const &peer_node) {
    g_servers.emplace_back(g_client_ctx, false);
    size_t idx = g_servers.size() - 1;
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
        struct timeval timeout = g_timeout;
        if (process_peer(idx, server, &timeout) == process_res::ERR) return -1;
    }

    if (!server.connected()) {
        ERROR("failed to finish handshake");
        server.close();
        return -1;
    }

    FINE("connected to peer in %s:%d", peer_node.addr, peer_node.port);

    return 0;
}

process_res process_peer(size_t idx, peer &p, struct timeval *timeout) {
    if (p.connected() && p.want_read()) {
        handle_received_message(idx, p);
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
    for (size_t idx = 0; idx < g_servers.size(); ++idx) {
        auto &server = g_servers[idx];
        if (server.connected() && server.want_read()) {
            handle_received_message(idx, server);
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

}  // namespace restart_rollback
}  // namespace register_sgx
