#include "reset_handler.h"
#include "handler_helpers.h"
#include "log.h"
#include "state_machine.h"

extern paxos_sgx::crash::StateMachine g_state_machine;

namespace paxos_sgx {
namespace crash {
namespace handler {

int reset_handler(peer &p, int64_t ticket) {
    LOG("reset request [%ld]", ticket);

    g_state_machine.reset();

    flatbuffers::FlatBufferBuilder builder;
    auto reset_res = paxos_sgx::crash::CreatePingResult(builder);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_reset, ticket,
        paxos_sgx::crash::Result_PingResult, reset_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
