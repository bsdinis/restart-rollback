/**
 * replica_handler.h
 *
 * handler for the replica operations
 */
#pragma once

#include <cstdint>
#include "call_map.h"
#include "crash_generated.h"
#include "peer.h"

namespace register_sgx {
namespace crash {
namespace handler {

// request handlers
int get_handler(peer &p, int64_t ticket, GetArgs const *args);
int get_timestamp_handler(peer &p, int64_t ticket,
                          GetTimestampArgs const *args);
int put_handler(peer &p, int64_t ticket, PutArgs const *args);

// response handlers
int get_resp_handler(peer &p, size_t peer_idx, int64_t ticket,
                     GetResult const *args);
int get_timestamp_resp_handler(peer &p, int64_t ticket,
                               GetTimestampResult const *args);
int put_resp_handler(peer &p, int64_t ticket, PutResult const *args);

// direct handlers: handlers for calls we execute ourselves
int get_resp_handler_action(GetCallContext &context, size_t peer_idx,
                            std::array<uint8_t, REGISTER_SIZE> const &value,
                            int64_t timestamp);
int get_timestamp_resp_handler_action(PutCallContext &context,
                                      int64_t timestamp);

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
