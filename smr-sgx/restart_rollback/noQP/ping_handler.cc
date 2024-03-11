#include "ping_handler.h"

#include "restart_rollback_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::restart_rollback::CreatePingResult(builder);
    auto result = paxos_sgx::restart_rollback::CreateBasicResponse(
        builder, paxos_sgx::restart_rollback::ReqType_ping, ticket,
        paxos_sgx::restart_rollback::Result_PingResult, ping_res.Union());
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
}  // namespace restart_rollback
}  // namespace paxos_sgx
