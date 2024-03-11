/**
 * handler_helpers.cc
 *
 * a handler helper
 */
#include <stdint.h>
#include "crash_generated.h"
#include "log.h"
#include "peer.h"

extern std::vector<peer> g_replica_list;
namespace register_sgx {
namespace crash {
namespace handler_helper {

int append_message(peer &p, flatbuffers::FlatBufferBuilder &&builder) {
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

int broadcast(flatbuffers::FlatBufferBuilder &&builder) {
    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    for (auto &peer : g_replica_list) {
        if (peer.append(&size, 1) == -1) {
            ERROR("failed to prepare message to send");
            return -1;
        } else if (peer.append(payload, size) == -1) {
            ERROR("failed to prepare message to send");
            return -1;
        }
    }

    return 0;
}

int broadcast_to(std::vector<size_t> const &indices,
                 flatbuffers::FlatBufferBuilder &&builder) {
    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    for (auto const idx : indices) {
        auto &peer = g_replica_list[idx];
        if (peer.append(&size, 1) == -1) {
            ERROR("failed to prepare message to send");
            return -1;
        } else if (peer.append(payload, size) == -1) {
            ERROR("failed to prepare message to send");
            return -1;
        }
    }

    return 0;
}

}  // namespace handler_helper
}  // namespace crash
}  // namespace register_sgx
