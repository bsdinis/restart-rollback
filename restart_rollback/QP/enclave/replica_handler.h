/**
 * replica_handler.h
 *
 * handler for the replica operations
 */
#pragma once

#include <cstdint>
#include "restart_rollback_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int replica_propose_handler(peer &p, int64_t ticket,
                            ReplicaPropose const *message);
int replica_accept_handler(peer &p, int64_t ticket,
                           ReplicaAccept const *message);
int replica_reject_handler(peer &p, int64_t ticket,
                           ReplicaReject const *message);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
