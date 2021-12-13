/**
 * client_handler.h
 *
 * handler for the client operations
 */
#pragma once

#include <cstdint>
#include "restart_rollback_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int client_fast_get_handler(peer &p, int64_t ticket, FastGetArgs const *args);
int client_operation_handler(peer &p, int64_t ticket,
                             OperationArgs const *args);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
