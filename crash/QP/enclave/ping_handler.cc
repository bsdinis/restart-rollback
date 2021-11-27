#include "ping_handler.h"

#include "crash_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::crash::CreatePingResult(builder);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_ping, ticket,
        paxos_sgx::crash::Result_PingResult, ping_res.Union());
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
}  // namespace crash
}  // namespace paxos_sgx
