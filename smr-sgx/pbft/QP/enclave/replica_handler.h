/**
 * replica_handler.h
 *
 * handler for the replica operations
 */
#pragma once

#include <cstdint>
#include "pbft_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int replica_pre_prepare_handler(peer &p, int64_t ticket,
                                ReplicaPrePrepare const *message);
int replica_prepare_handler(peer &p, int64_t ticket,
                            ReplicaPrepare const *message);
int replica_commit_handler(peer &p, int64_t ticket,
                           ReplicaCommit const *message);

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
