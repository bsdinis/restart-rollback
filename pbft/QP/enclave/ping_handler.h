/**
 * ping_handler.h
 *
 * handler for the ping operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket);
int replica_ping_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
