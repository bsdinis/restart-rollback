/**
 * setup.c
 */

#include "setup.h"

#include <stdlib.h>
#include <string.h>
#include "ssl_util.h"

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

namespace {
int listen_sock_ = -1;
SSL_CTX* ssl_server_ctx;
bool closed_ = false;

char const* server_crt = "certs/server.crt";
char const* server_key = "certs/server.key";

}  // anonymous namespace

// ========================================

namespace epidemics {
namespace basicQP {
namespace setup {

using namespace epidemics::basicQP;

void setup(config_t* conf, ssize_t idx) {
    config_node_t& node = conf->nodes[idx];
    char addr[50];
    ocall_net_get_my_ipv4_addr(addr, 50);

    if (strncmp(addr, node.addr, 50) != 0) {
        KILL("cannot listen on IP %s, my IP is %s", node.addr, addr);
    }

    sgx_status_t error =
        ocall_net_start_listen_socket(&listen_sock_, addr, (uint16_t)node.port);
    if (error != SGX_SUCCESS) {
        sgx_perror(error);
        ocall_sgx_exit(EXIT_FAILURE);
    }
    if (listen_sock_ <= 0)
        KILL("failed to setup the listening socket on %s:%d", node.addr,
             node.port);

    if (ssl::init_server_ssl_ctx(&ssl_server_ctx) != 0)
        KILL("failed to init SSL server");

    if (ssl::load_certificates(ssl_server_ctx, server_crt, server_key) != 0)
        KILL("failed to load certificates for server SSL");
}

void close() {
    INFO("closing the enclave");
    int ret = 0;
    if (listen_sock_ > 0) ocall_sgx_close(&ret, listen_sock_);
    if (ssl_server_ctx) SSL_CTX_free(ssl_server_ctx);
    listen_sock_ = -1;
    ssl_server_ctx = nullptr;
    closed_ = true;
}

SSL_CTX* ssl_ctx() { return ssl_server_ctx; }
int listen_sock() { return listen_sock_; }
bool closed() { return closed_; }

}  // namespace setup
}  // namespace basicQP
}  // namespace epidemics
