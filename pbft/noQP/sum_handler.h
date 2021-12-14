/**
 * sum_handler.h
 *
 * handler for the sum operation
 */
#pragma once

#include <cstdint>
#include <vector>
#include "pbft_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec);

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
