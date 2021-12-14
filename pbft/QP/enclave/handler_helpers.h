/**
 * handler_helpers.h
 *
 * a handler helper
 */
#pragma once

#include <stdint.h>
#include "pbft_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler_helper {

int append_result(peer &p, flatbuffers::FlatBufferBuilder &&response);

}  // namespace handler_helper
}  // namespace pbft
}  // namespace paxos_sgx
