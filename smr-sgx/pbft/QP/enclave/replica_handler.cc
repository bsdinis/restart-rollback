#include "replica_handler.h"

#include "call_map.h"
#include "log.h"
#include "op_log.h"
#include "pbft_generated.h"
#include "replicas.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::pbft::CallMap g_call_map;
extern paxos_sgx::pbft::OpLog g_log;
extern paxos_sgx::pbft::StateMachine g_state_machine;

namespace {
int broadcast_prepare(int64_t ticket,
                      paxos_sgx::pbft::ReplicaPrePrepare const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto op = paxos_sgx::pbft::CreateOperationArgs(
        builder, message->operation()->account(), message->operation()->to(),
        message->operation()->amount());
    auto prepare = paxos_sgx::pbft::CreateReplicaPrepare(
        builder, message->slot_number(), op);
    auto msg = paxos_sgx::pbft::CreateMessage(
        builder, paxos_sgx::pbft::MessageType_replica_prepare, ticket,
        paxos_sgx::pbft::BasicMessage_ReplicaPrepare, prepare.Union());
    builder.Finish(msg);

    return paxos_sgx::pbft::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}
}  // namespace

namespace paxos_sgx {
namespace pbft {
namespace handler {

int replica_pre_prepare_handler(
    peer &p, int64_t ticket,
    paxos_sgx::pbft::ReplicaPrePrepare const *message) {
    LOG("replica pre prepare request [%ld]: slot %ld", ticket,
        message->slot_number());

    g_call_map.add_call(message->slot_number(), ticket);

    if (g_log.add_op(message->slot_number(), message->operation())) {
        if (replicas::add_prepare(ticket, message->slot_number(),
                                  message->operation())) {
            return broadcast_prepare(ticket, message);
        }
    }

    ERROR(
        "failed to pre prepare operation: should never happen without view "
        "changes (maybe the operation changed)");
    return -1;
}

int replica_prepare_handler(peer &p, int64_t ticket,
                            paxos_sgx::pbft::ReplicaPrepare const *message) {
    LOG("replica prepare request [%ld]: slot %ld", ticket,
        message->slot_number());

    if (!paxos_sgx::pbft::replicas::add_prepare(ticket, message->slot_number(),
                                                message->operation())) {
        ERROR("failed to prepare operation: the operation changed");
        return -1;
    }

    return 0;
}

int replica_commit_handler(peer &p, int64_t ticket,
                           paxos_sgx::pbft::ReplicaCommit const *message) {
    LOG("replica commit request [%ld]: slot %ld", ticket,
        message->slot_number());

    paxos_sgx::pbft::replicas::add_commit(message->slot_number(),
                                          message->operation());
    return 0;
}

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
