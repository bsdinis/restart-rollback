/**
 * proxy_handler.h
 *
 * handler for the client operations
 */
#pragma once

#include <cstdint>
#include "crash_generated.h"
#include "peer.h"

namespace register_sgx {
namespace crash {
namespace handler {

int proxy_get_handler(peer &p, int64_t ticket, GetArgs const *args);
int proxy_put_handler(peer &p, int64_t ticket, ProxyPutArgs const *args);

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
