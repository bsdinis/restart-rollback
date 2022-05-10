/**
 * ping_handler.h
 *
 * handler for the ping operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace register_sgx {
namespace byzantine {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket);
int replica_ping_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
