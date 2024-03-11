/**
 * client_handler.h
 *
 * handler for the client operations
 */
#pragma once

#include <cstdint>
#include "pbft_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int client_fast_get_handler(peer &p, int64_t ticket, FastGetArgs const *args);
int client_operation_handler(peer &p, int64_t ticket,
                             OperationArgs const *args);

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
