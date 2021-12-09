/**
 * replica_handler.h
 *
 * handler for the replica operations
 */
#pragma once

#include <cstdint>
#include "crash_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int replica_fast_get_handler(peer &p, int64_t ticket, FastGetArgs const *args);
int replica_fast_get_resp_handler(peer &p, int64_t ticket,
                                  ReplicaFastGetResult const *resp);
int replica_propose_handler(peer &p, int64_t ticket,
                            ReplicaPropose const *message);
int replica_accept_handler(peer &p, int64_t ticket,
                           ReplicaAccept const *message);
int replica_reject_handler(peer &p, int64_t ticket,
                           ReplicaReject const *message);

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
