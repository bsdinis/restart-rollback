/**
 * handler_helpers.h
 *
 * a handler helper
 */
#pragma once

#include <stdint.h>
#include "crash_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace crash {
namespace handler_helper {

int append_result(peer &p, flatbuffers::FlatBufferBuilder &&response);

}  // namespace handler_helper
}  // namespace crash
}  // namespace paxos_sgx
