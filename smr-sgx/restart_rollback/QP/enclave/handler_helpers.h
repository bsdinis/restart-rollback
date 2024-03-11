/**
 * handler_helpers.h
 *
 * a handler helper
 */
#pragma once

#include <stdint.h>
#include "peer.h"
#include "restart_rollback_generated.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler_helper {

int append_result(peer &p, flatbuffers::FlatBufferBuilder &&response);

}  // namespace handler_helper
}  // namespace restart_rollback
}  // namespace paxos_sgx
