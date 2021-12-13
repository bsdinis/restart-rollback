#include "call_map.h"
#include "restart_rollback_generated.h"
#include "handler_helpers.h"
#include "log.h"

namespace paxos_sgx {
namespace restart_rollback {

void CallMap::add_call(size_t slot_n, peer* client, int64_t ticket) {
    m_map.emplace(slot_n, CallContext(client, ticket));
}

void CallMap::resolve_call(size_t slot_n, int64_t account, int64_t amount,
                           bool success) {
    auto it = m_map.find(slot_n);
    if (it == std::end(m_map)) {
        ERROR("failed to find call for slot %zu", slot_n);
        return;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto client_operation_res = paxos_sgx::restart_rollback::CreateOperationResult(
        builder, account, amount, success);
    auto result = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_client_operation_resp,
        it->second.m_ticket, paxos_sgx::restart_rollback::BasicMessage_OperationResult,
        client_operation_res.Union());
    builder.Finish(result);

    if (paxos_sgx::restart_rollback::handler_helper::append_result(
            *it->second.m_client, std::move(builder)) == -1) {
        ERROR("failed to find send response for slot %zu to client", slot_n);
    }

    m_map.erase(it);
}

}  // namespace restart_rollback
}  // namespace paxos_sgx
