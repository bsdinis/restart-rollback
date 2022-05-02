/**
 * client_handler.h
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

int client_get_handler(peer &p, int64_t ticket, GetArgs const *args);
int client_get_timestamp_handler(peer &p, int64_t ticket,
                                 GetTimestampArgs const *args);
int client_put_handler(peer &p, int64_t ticket, PutArgs const *args);

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
