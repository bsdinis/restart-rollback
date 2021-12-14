#include "client_handler.h"

#include "call_map.h"
#include "handler_helpers.h"
#include "log.h"
#include "op_log.h"
#include "pbft_generated.h"
#include "replicas.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::pbft::CallMap g_call_map;
extern paxos_sgx::pbft::StateMachine g_state_machine;
extern paxos_sgx::pbft::OpLog g_log;

namespace paxos_sgx {
namespace pbft {
namespace handler {

int client_fast_get_handler(peer &p, int64_t ticket,
                            paxos_sgx::pbft::FastGetArgs const *args) {
    LOG("client fast get request [%ld]: account %ld", ticket, args->account());

    flatbuffers::FlatBufferBuilder builder;
    auto client_fast_get_res = paxos_sgx::pbft::CreateFastGetResult(
        builder, args->account(), g_state_machine.get(args->account()),
        g_log.execution_cursor(), g_log.committed_cursor());
    auto result = paxos_sgx::pbft::CreateMessage(
        builder, paxos_sgx::pbft::MessageType_client_fast_get_resp, ticket,
        paxos_sgx::pbft::BasicMessage_FastGetResult,
        client_fast_get_res.Union());
    builder.Finish(result);

    return paxos_sgx::pbft::handler_helper::append_result(p,
                                                          std::move(builder));
}

int client_operation_handler(peer &p, int64_t ticket,
                             paxos_sgx::pbft::OperationArgs const *args) {
    LOG("client operation request [%ld]", ticket);

    size_t slot_n = g_log.pre_prepare_op(args);
    size_t execution_slot = slot_n;

    g_call_map.add_call(slot_n, ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto op_args = paxos_sgx::pbft::CreateOperationArgs(
        builder, args->account(), args->to(), args->amount());
    auto pre_prepare =
        paxos_sgx::pbft::CreateReplicaPrePrepare(builder, slot_n, op_args);
    auto message = paxos_sgx::pbft::CreateMessage(
        builder, paxos_sgx::pbft::MessageType_replica_pre_prepare, ticket,
        paxos_sgx::pbft::BasicMessage_ReplicaPrePrepare, pre_prepare.Union());
    builder.Finish(message);

    return paxos_sgx::pbft::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
