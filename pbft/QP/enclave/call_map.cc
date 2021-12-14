#include "call_map.h"
#include "handler_helpers.h"
#include "log.h"
#include "pbft_generated.h"
#include "peer.h"

extern std::vector<peer> g_client_list;

namespace paxos_sgx {
namespace pbft {

void CallMap::add_call(size_t slot_n, int64_t ticket) {
    m_map.emplace(slot_n, ticket);
}

void CallMap::resolve_call(size_t slot_n, int64_t account, int64_t amount,
                           bool success) {
    auto it = m_map.find(slot_n);
    if (it == std::end(m_map)) {
        ERROR("failed to find call for slot %zu", slot_n);
        return;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto client_operation_res = paxos_sgx::pbft::CreateOperationResult(
        builder, account, amount, success);
    auto result = paxos_sgx::pbft::CreateMessage(
        builder, paxos_sgx::pbft::MessageType_client_operation_resp, it->second,
        paxos_sgx::pbft::BasicMessage_OperationResult,
        client_operation_res.Union());
    builder.Finish(result);

    for (auto &client : g_client_list) {
        if (paxos_sgx::pbft::handler_helper::append_result(
                client, std::move(builder)) == -1) {
            ERROR("failed to find send response for slot %zu to client",
                  slot_n);
        }
    }

    m_map.erase(it);
}

}  // namespace pbft
}  // namespace paxos_sgx
