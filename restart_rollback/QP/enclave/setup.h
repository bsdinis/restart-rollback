/**
 * setup.h
 *
 * setup routines for gatekeeper
 *
 * should be called onde and without parameters (ergo, this header is very
 * simple)
 */

#ifndef __SETUP_HEADER__
#define __SETUP_HEADER__

#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "enclave_t.h"

#include <vector>
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace setup {

void setup(config_t* conf, ssize_t idx, void* file_mapping,
           size_t mapping_size);
void close();

SSL_CTX* ssl_ctx();
int client_listen_sock();
int replica_listen_sock();
bool closed();

size_t write_quorum_size();
size_t read_quorum_size(size_t s);
size_t max_quorum_size(size_t suspicions);
bool is_suspicious();
bool is_leader();

}  // namespace setup
}  // namespace restart_rollback
}  // namespace paxos_sgx

#endif  //__SETUP_HEADER__
