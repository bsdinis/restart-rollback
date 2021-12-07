/**
 * client_handler.h
 *
 * handler for the client operations
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

int client_fast_get_handler(peer &p, int64_t ticket,
                            ClientFastGetArgs const *args);
int client_operation_handler(peer &p, int64_t ticket,
                             OperationArgs const *args);

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
