#include "replicas.h"
#include "call_map.h"
#include "log.h"
#include "op_log.h"
#include "peer.h"
#include "setup.h"
#include "state_machine.h"

extern std::vector<peer> g_replica_list;
extern paxos_sgx::pbft::CallMap g_call_map;
extern paxos_sgx::pbft::OpLog g_log;
extern paxos_sgx::pbft::StateMachine g_state_machine;

namespace {
int broadcast_commit(int64_t ticket, size_t slot_n,
                     paxos_sgx::pbft::OperationArgs const *op) {
    flatbuffers::FlatBufferBuilder builder;
    auto operation = paxos_sgx::pbft::CreateOperationArgs(
        builder, op->account(), op->to(), op->amount());
    auto commit =
        paxos_sgx::pbft::CreateReplicaPrepare(builder, slot_n, operation);
    auto msg = paxos_sgx::pbft::CreateMessage(
        builder, paxos_sgx::pbft::MessageType_replica_commit, ticket,
        paxos_sgx::pbft::BasicMessage_ReplicaCommit, commit.Union());
    builder.Finish(msg);

    return paxos_sgx::pbft::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}
}  // namespace

namespace paxos_sgx {
namespace pbft {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size) {
    int ret = 0;
    for (auto &replica : g_replica_list) {
        ret |= replica.append((uint8_t *)&size, sizeof(size_t));
        ret |= replica.append(message, size);
    };

    return ret;
}

bool add_prepare(int64_t ticket, size_t slot_number,
                 paxos_sgx::pbft::OperationArgs const *op) {
    if (!g_log.check_op(slot_number, op)) {
        return false;
    }
    bool was_prepared = g_log.is_prepared(slot_number);

    g_log.add_prepare(slot_number);

    if (g_log.is_prepared(slot_number) && !was_prepared) {
        g_log.add_commit(slot_number);
        if (broadcast_commit(ticket, slot_number, op) == -1) {
            ERROR("failed to broadcast commit for slot %zu", slot_number);
        }
    }

    return true;
}

void add_commit(size_t slot_number, paxos_sgx::pbft::OperationArgs const *op) {
    if (!g_log.check_op(slot_number, op)) {
        ERROR("failed to commit operation: the operation changed");
    }

    g_log.add_commit(slot_number);
    while (g_log.can_execute(slot_number)) {
        execute(slot_number);
        slot_number += 1;
    }
}

void execute(size_t slot_number) {
    int64_t account = 0;
    int64_t amount = 0;
    bool success = g_state_machine.execute(g_log.get_operation(slot_number),
                                           account, amount);
    g_log.executed(slot_number);
    g_call_map.resolve_call(slot_number, account, amount, success);
}

}  // namespace replicas
}  // namespace pbft
}  // namespace paxos_sgx
