/**
 * setup.c
 */

#include "setup.h"

#include <stdlib.h>
#include <string.h>
#include "ssl_util.h"
#include "state_machine.h"

#include "error.h"

#include <unistd.h>

#include "log.h"
#include "peer.h"

#include "handlers.h"

#include <sgx_trts_exception.h>
#include "enclave_t.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

paxos_sgx::crash::StateMachine g_state_machine;
std::vector<peer> g_client_list;
std::vector<peer> g_replica_list;

namespace {

int client_listen_sock_ = -1;
int replica_listen_sock_ = -1;
SSL_CTX* ssl_server_ctx;
SSL_CTX* ssl_client_ctx;
bool closed_ = false;

char const* server_crt = "certs/server.crt";
char const* server_key = "certs/server.key";

int connect_replica(config_node_t const& peer_config) {
    g_replica_list.emplace_back(ssl_client_ctx, false);
    peer& p = *(std::end(g_replica_list) - 1);

    char addr[50];
    memset(addr, 0, sizeof(addr));
    strncpy(addr, peer_config.addr, 49);

    if (p.connect(addr, peer_config.port * 2) != 0) {
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
        if (p.recv() == -1) {
            ERROR("failed to recv on hanshake");
            p.close();
            return -1;
        }
    }

    INFO("connected to peer on %s:%d", peer_config.addr, peer_config.port * 2);
    return 0;
}

int connect_to_all_replicas(config_t const* config, ssize_t my_idx) {
    for (ssize_t idx = 0; idx < my_idx; ++idx) {
        if (connect_replica(config->nodes[idx]) != 0) {
            ERROR("failed to connect to replica on %s:%d",
                  config->nodes[idx].addr, config->nodes[idx].port * 2);
            return -1;
        }
    }

    return 0;
}

}  // anonymous namespace

// ========================================

namespace paxos_sgx {
namespace crash {
namespace setup {

using namespace paxos_sgx::crash;

void setup(config_t* conf, ssize_t idx) {
    config_node_t& node = conf->nodes[idx];
    char addr[50];
    ocall_net_get_my_ipv4_addr(addr, 50);

    if (strncmp(addr, node.addr, 50) != 0) {
        KILL("cannot listen on IP %s, my IP is %s", node.addr, addr);
    }

    sgx_status_t error = ocall_net_start_listen_socket(
        &client_listen_sock_, addr, (uint16_t)node.port);
    if (error != SGX_SUCCESS) {
        sgx_perror(error);
        ocall_sgx_exit(EXIT_FAILURE);
    }
    if (client_listen_sock_ <= 0) {
        KILL("failed to setup the listening socket on %s:%d", node.addr,
             node.port);
    }

    error = ocall_net_start_listen_socket(&replica_listen_sock_, addr,
                                          (uint16_t)(node.port * 2));
    if (error != SGX_SUCCESS) {
        sgx_perror(error);
        ocall_sgx_exit(EXIT_FAILURE);
    }
    if (replica_listen_sock_ <= 0) {
        KILL("failed to setup the listening socket on %s:%d", node.addr,
             node.port * 2);
    }

    if (ssl::init_server_ssl_ctx(&ssl_server_ctx) != 0) {
        KILL("failed to init SSL server");
    }

    if (ssl::init_client_ssl_ctx(&ssl_client_ctx) != 0) {
        KILL("failed to init SSL client");
    }

    if (ssl::load_certificates(ssl_server_ctx, server_crt, server_key) != 0) {
        KILL("failed to load certificates for server SSL");
    }
    if (ssl::load_certificates(ssl_client_ctx, server_crt, server_key) != 0) {
        KILL("failed to load certificates for client SSL");
    }

    if (connect_to_all_replicas(conf, idx) != 0) {
        KILL("failed to connect to replicas");
    }
}

void close() {
    INFO("closing the enclave");
    int ret = 0;
    if (client_listen_sock_ > 0) ocall_sgx_close(&ret, client_listen_sock_);
    if (replica_listen_sock_ > 0) ocall_sgx_close(&ret, replica_listen_sock_);
    if (ssl_server_ctx) SSL_CTX_free(ssl_server_ctx);
    if (ssl_client_ctx) SSL_CTX_free(ssl_client_ctx);
    client_listen_sock_ = -1;
    replica_listen_sock_ = -1;
    ssl_server_ctx = nullptr;
    closed_ = true;
}

SSL_CTX* ssl_ctx() { return ssl_server_ctx; }
int client_listen_sock() { return client_listen_sock_; }
int replica_listen_sock() { return replica_listen_sock_; }
bool closed() { return closed_; }

}  // namespace setup
}  // namespace crash
}  // namespace paxos_sgx
