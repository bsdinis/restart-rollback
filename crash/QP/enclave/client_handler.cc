

#include "client_handler.h"
#include "call_map.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "op_log.h"
#include "replicas.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::crash::CallMap g_call_map;
extern paxos_sgx::crash::StateMachine g_state_machine;
extern paxos_sgx::crash::OpLog g_log;

namespace paxos_sgx {
namespace crash {
namespace handler {

int client_fast_get_handler(peer &p, int64_t ticket,
                            paxos_sgx::crash::FastGetArgs const *args) {
    LOG("client fast get request [%ld]: account %ld", ticket, args->account());

    flatbuffers::FlatBufferBuilder builder;
    auto client_fast_get_res = paxos_sgx::crash::CreateFastGetResult(
        builder, args->account(), g_state_machine.get(args->account()),
        g_log.execution_cursor(), g_log.accepted_cursor());
    auto result = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_client_fast_get_resp, ticket,
        paxos_sgx::crash::BasicMessage_FastGetResult,
        client_fast_get_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

int client_operation_handler(peer &p, int64_t ticket,
                             paxos_sgx::crash::OperationArgs const *args) {
    LOG("client operation request [%ld]", ticket);

    size_t slot_n = g_log.propose_op(args);
    while (g_log.get_accepts(slot_n) >=
               paxos_sgx::crash::setup::quorum_size() &&
           g_log.execution_cursor() == slot_n - 1) {
        replicas::execute(slot_n);
    }

    g_call_map.add_call(slot_n, &p, ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto op_args = paxos_sgx::crash::CreateOperationArgs(
        builder, args->account(), args->to(), args->amount());
    auto propose =
        paxos_sgx::crash::CreateReplicaPropose(builder, op_args, slot_n);
    auto message = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_replica_propose, ticket,
        paxos_sgx::crash::BasicMessage_ReplicaPropose, propose.Union());
    builder.Finish(message);

    return paxos_sgx::crash::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
