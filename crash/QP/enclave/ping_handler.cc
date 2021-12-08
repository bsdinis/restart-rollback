#include "ping_handler.h"
#include "handler_helpers.h"
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

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
