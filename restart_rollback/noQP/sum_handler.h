/**
 * sum_handler.h
 *
 * handler for the sum operation
 */
#pragma once

#include <cstdint>
#include <vector>
#include "restart_rollback_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
