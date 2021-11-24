#include "ping_handler.h"

#include "basicQP_resp_generated.h"
#include "log.h"

namespace epidemics {
namespace basicQP {
namespace handler {

int ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = epidemics::basicQP::CreatePingResult(builder);
    auto result = epidemics::basicQP::CreateBasicResponse(
        builder, epidemics::basicQP::ReqType_ping, ticket,
        epidemics::basicQP::Result_PingResult, ping_res.Union());
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
}  // namespace basicQP
}  // namespace epidemics
