/**
 * reset_handler.h
 *
 * handler for the reset operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int reset_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx