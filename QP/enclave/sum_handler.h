/**
 * sum_handler.h
 *
 * handler for the sum operation
 */
#pragma once

#include <cstdint>
#include <vector>
#include "basicQP_generated.h"
#include "peer.h"

namespace epidemics {
namespace basicQP {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec);

}  // namespace handler
}  // namespace basicQP
}  // namespace epidemics
