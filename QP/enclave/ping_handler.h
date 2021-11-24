/**
 * ping_handler.h
 *
 * handler for the ping operation
 */
#pragma once

#include <cstdint>
#include "peer.h"

namespace epidemics {
namespace basicQP {
namespace handler {

int ping_handler(peer &p, int64_t ticket);

}  // namespace handler
}  // namespace basicQP
}  // namespace epidemics
