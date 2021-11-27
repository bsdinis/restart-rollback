/**
 * sum_handler.h
 *
 * handler for the sum operation
 */
#pragma once

#include <cstdint>
#include <vector>
#include "crash_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec);

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
