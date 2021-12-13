#include "reset_handler.h"
#include "handler_helpers.h"
#include "log.h"
#include "state_machine.h"

extern paxos_sgx::restart_rollback::StateMachine g_state_machine;

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int reset_handler(peer &p, int64_t ticket) {
    LOG("reset request [%ld]", ticket);

    g_state_machine.reset();

    flatbuffers::FlatBufferBuilder builder;
    auto reset_res = paxos_sgx::restart_rollback::CreateEmpty(builder);
    auto result = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_reset_resp, ticket,
        paxos_sgx::restart_rollback::BasicMessage_Empty, reset_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    return paxos_sgx::restart_rollback::handler_helper::append_result(p,
                                                           std::move(builder));
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
