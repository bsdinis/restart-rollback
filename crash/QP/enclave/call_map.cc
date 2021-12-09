#include "call_map.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "log.h"

namespace paxos_sgx {
namespace crash {

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
    auto result = paxos_sgx::crash::CreateOperationResult(builder, account,
                                                          amount, success);
    auto message = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_client_operation_resp,
        it->second.m_ticket, paxos_sgx::crash::BasicMessage_OperationResult,
        result.Union());
    builder.Finish(result);

    LOG("replying to ticket %zd (peer %p)", it->second.m_ticket,
        it->second.m_client);
    if (paxos_sgx::crash::handler_helper::append_result(
            *it->second.m_client, std::move(builder)) == -1) {
        ERROR("failed to find send response for slot %zu to client", slot_n);
    }

    m_map.erase(it);
}

}  // namespace crash
}  // namespace paxos_sgx
