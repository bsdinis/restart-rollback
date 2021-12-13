/**
 * ping_handler.h
 *
 * handler for the ping operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int close_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
