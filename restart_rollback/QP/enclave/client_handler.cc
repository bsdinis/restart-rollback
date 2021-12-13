

#include "client_handler.h"
#include "call_map.h"
#include "handler_helpers.h"
#include "log.h"
#include "op_log.h"
#include "replicas.h"
#include "restart_rollback_generated.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::restart_rollback::CallMap g_call_map;
extern paxos_sgx::restart_rollback::StateMachine g_state_machine;
extern paxos_sgx::restart_rollback::OpLog g_log;

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int client_fast_get_handler(
    peer &p, int64_t ticket,
    paxos_sgx::restart_rollback::FastGetArgs const *args) {
    LOG("client fast get request [%ld]: account %ld", ticket, args->account());

    flatbuffers::FlatBufferBuilder builder;
    auto client_fast_get_res = paxos_sgx::restart_rollback::CreateFastGetResult(
        builder, args->account(), g_state_machine.get(args->account()),
        g_log.execution_cursor(), g_log.accepted_cursor(), g_log.last_seen(),
        paxos_sgx::restart_rollback::setup::is_suspicious());
    auto result = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_client_fast_get_resp,
        ticket, paxos_sgx::restart_rollback::BasicMessage_FastGetResult,
        client_fast_get_res.Union());
    builder.Finish(result);

    return paxos_sgx::restart_rollback::handler_helper::append_result(
        p, std::move(builder));
}

int client_operation_handler(
    peer &p, int64_t ticket,
    paxos_sgx::restart_rollback::OperationArgs const *args) {
    LOG("client operation request [%ld]", ticket);

    size_t slot_n = g_log.propose_op(
        args, paxos_sgx::restart_rollback::setup::is_suspicious());
    size_t execution_slot = slot_n;
    while (g_log.can_execute(execution_slot)) {
        replicas::execute(execution_slot);
        execution_slot += 1;
    }

    g_call_map.add_call(slot_n, &p, ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto op_args = paxos_sgx::restart_rollback::CreateOperationArgs(
        builder, args->account(), args->to(), args->amount());
    auto propose = paxos_sgx::restart_rollback::CreateReplicaPropose(
        builder, op_args, slot_n,
        paxos_sgx::restart_rollback::setup::is_suspicious());
    auto message = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_replica_propose,
        ticket, paxos_sgx::restart_rollback::BasicMessage_ReplicaPropose,
        propose.Union());
    builder.Finish(message);

    return paxos_sgx::restart_rollback::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
