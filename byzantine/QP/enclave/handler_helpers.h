/**
 * handler_helpers.h
 *
 * a handler helper
 */
#pragma once

#include <stdint.h>
#include "byzantine_generated.h"
#include "peer.h"

namespace register_sgx {
namespace byzantine {
namespace handler_helper {

int append_message(peer &p, flatbuffers::FlatBufferBuilder &&message);
int broadcast(flatbuffers::FlatBufferBuilder &&message);
int broadcast_to(std::vector<size_t> const &indices,
                 flatbuffers::FlatBufferBuilder &&message);

}  // namespace handler_helper
}  // namespace byzantine
}  // namespace register_sgx
