#include "ping_handler.h"

#include "pbft_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::pbft::CreatePingResult(builder);
    auto result = paxos_sgx::pbft::CreateBasicResponse(
        builder, paxos_sgx::pbft::ReqType_ping, ticket,
        paxos_sgx::pbft::Result_PingResult, ping_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    FINE("response has %zu B", size);
    if (p.append(&size, 1) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    } else if (p.append(payload, size) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    }
    return 0;
}

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
