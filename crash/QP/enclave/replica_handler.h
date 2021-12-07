/**
 * replica_handler.h
 *
 * handler for the replica operations
 */
#pragma once

#include <cstdint>
#include "crash_generated.h"
#include "crash_req_generated.h"
#include "crash_resp_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int replica_fast_get_handler(peer &p, int64_t ticket,
                             ReplicaFastGetArgs const *args);
int replica_propose_handler(peer &p, int64_t ticket,
                            ReplicaProposeArgs const *args);
int replica_accept_handler(peer &p, int64_t ticket,
                           ReplicaAcceptArgs const *args);

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
