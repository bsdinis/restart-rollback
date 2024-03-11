/**
 * close_handler.h
 *
 * handler for the close operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace teems {
namespace handler {

int close_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace teems
