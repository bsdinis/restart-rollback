/**
 * setup.c
 */

#include "setup.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sgx_trts_exception.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "enclave_t.h"

#include "call_map.h"
#include "error.h"
#include "handlers.h"
#include "log.h"
#include "op_log.h"
#include "peer.h"
#include "ssl_util.h"
#include "state_machine.h"

paxos_sgx::restart_rollback::CallMap g_call_map;
paxos_sgx::restart_rollback::StateMachine g_state_machine;
paxos_sgx::restart_rollback::OpLog g_log;
std::vector<peer> g_client_list;
std::vector<peer> g_replica_list;
void* g_persistent_array = NULL;
size_t g_persistent_array_size = 0;

namespace {

int g_my_idx = -1;
int client_listen_sock_ = -1;
int replica_listen_sock_ = -1;
SSL_CTX* ssl_server_ctx;
SSL_CTX* ssl_client_ctx;
bool closed_ = false;

size_t g_crash_faults = 0;
size_t g_rollback_faults = 0;
bool g_is_fresh = false;

char const* server_crt = "certs/server.crt";
char const* server_key = "certs/server.key";

int connect_replica(config_node_t const& peer_config) {
    FINE("connecting to %s:%d", peer_config.addr, peer_config.port * 2);
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
        if (p.recv(true /* ignoreEOF */) == -1) {
            ERROR("failed to recv on hanshake");
            p.close();
            return -1;
        }
    }

    INFO("connected to peer on %s:%d", peer_config.addr, peer_config.port * 2);

    FINE("processed: %zu", p.buffer().size());
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
namespace restart_rollback {
namespace setup {

using namespace paxos_sgx::restart_rollback;

void setup(config_t* conf, ssize_t idx, void* file_mapping,
           size_t mapping_size) {
    g_my_idx = idx;

    g_persistent_array = file_mapping;
    g_persistent_array_size = mapping_size;
    g_rollback_faults = conf->r;
    g_crash_faults = conf->f;

    config_node_t const& node = conf->nodes[idx];
    g_is_fresh = node.fresh;
    char addr[50];
    ocall_net_get_my_ipv4_addr(addr, 50);

    g_replica_list.reserve(conf->size);
    g_client_list.reserve(100);

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

size_t write_quorum_size() {
    return std::max<size_t>(g_rollback_faults, g_crash_faults) + 1;
}
size_t read_quorum_size(size_t suspicions) {
    return std::min<size_t>(g_rollback_faults, suspicions) + g_crash_faults + 1;
}
size_t max_quorum_size(size_t suspicions) {
    return std::max<size_t>(write_quorum_size(), read_quorum_size(suspicions));
}
bool is_leader() { return g_my_idx == 0; }
bool is_suspicious() { return !g_is_fresh; }

}  // namespace setup
}  // namespace restart_rollback
}  // namespace paxos_sgx
