/**
 * proxy_handler.h
 *
 * handler for the client operations
 */
#pragma once

#include <cstdint>
#include "peer.h"
#include "restart_rollback_generated.h"

namespace register_sgx {
namespace restart_rollback {
namespace handler {

int proxy_get_handler(peer &p, int64_t ticket, GetArgs const *args);
int proxy_put_handler(peer &p, int64_t ticket, ProxyPutArgs const *args);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
