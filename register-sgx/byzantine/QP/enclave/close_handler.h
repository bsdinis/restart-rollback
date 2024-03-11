/**
 * close_handler.h
 *
 * handler for the close operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace register_sgx {
namespace byzantine {
namespace handler {

int close_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
