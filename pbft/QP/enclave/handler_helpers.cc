/**
 * handler_helpers.cc
 *
 * a handler helper
 */
#include <stdint.h>
#include "log.h"
#include "pbft_generated.h"
#include "peer.h"

namespace paxos_sgx {
namespace pbft {
namespace handler_helper {

int append_result(peer &p, flatbuffers::FlatBufferBuilder &&builder) {
    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (p.append(&size, 1) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    } else if (p.append(payload, size) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    }

    return 0;
}

}  // namespace handler_helper
}  // namespace pbft
}  // namespace paxos_sgx
