#include "replica_handler.h"

#include "restart_rollback_generated.h"
#include "log.h"
#include "op_log.h"
#include "replicas.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::restart_rollback::OpLog g_log;
extern paxos_sgx::restart_rollback::StateMachine g_state_machine;

namespace {
int broadcast_accept(int64_t ticket,
                     paxos_sgx::restart_rollback::ReplicaPropose const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto accept =
        paxos_sgx::restart_rollback::CreateReplicaAccept(builder, message->slot_number());
    auto msg = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_replica_accept, ticket,
        paxos_sgx::restart_rollback::BasicMessage_ReplicaAccept, accept.Union());
    builder.Finish(msg);

    return paxos_sgx::restart_rollback::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}

int broadcast_reject(int64_t ticket,
                     paxos_sgx::restart_rollback::ReplicaPropose const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto reject =
        paxos_sgx::restart_rollback::CreateReplicaReject(builder, message->slot_number());
    auto msg = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_replica_reject, ticket,
        paxos_sgx::restart_rollback::BasicMessage_ReplicaReject, reject.Union());
    builder.Finish(msg);

    return paxos_sgx::restart_rollback::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}
}  // namespace

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int replica_propose_handler(peer &p, int64_t ticket,
                            paxos_sgx::restart_rollback::ReplicaPropose const *message) {
    LOG("replica propose request [%ld]: slot %ld", ticket,
        message->slot_number());

    if (g_log.add_op(message->slot_number(), message->operation_args())) {
        replicas::add_accept(message->slot_number());
        return broadcast_accept(ticket, message);
    }

    return broadcast_reject(ticket, message);
}

int replica_accept_handler(peer &p, int64_t ticket,
                           paxos_sgx::restart_rollback::ReplicaAccept const *message) {
    LOG("replica accept request [%ld]: slot %ld", ticket,
        message->slot_number());

    paxos_sgx::restart_rollback::replicas::add_accept(message->slot_number());
    return 0;
}

int replica_reject_handler(peer &p, int64_t ticket,
                           paxos_sgx::restart_rollback::ReplicaReject const *message) {
    LOG("replica reject request [%ld]: slot %ld", ticket,
        message->slot_number());
    KILL("Without a view change, there should never be a rejection");
    return 0;
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
