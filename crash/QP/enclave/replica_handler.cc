#include "replica_handler.h"

#include "crash_generated.h"
#include "log.h"
#include "op_log.h"
#include "replicas.h"
#include "setup.h"
#include "state_machine.h"

extern paxos_sgx::crash::OpLog g_log;
extern paxos_sgx::crash::StateMachine g_state_machine;

namespace {
int broadcast_accept(int64_t ticket,
                     paxos_sgx::crash::ReplicaPropose const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto accept =
        paxos_sgx::crash::CreateReplicaAccept(builder, message->slot_number());
    auto msg = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_replica_accept, ticket,
        paxos_sgx::crash::BasicMessage_ReplicaAccept, accept.Union());
    builder.Finish(msg);

    return paxos_sgx::crash::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}

int broadcast_reject(int64_t ticket,
                     paxos_sgx::crash::ReplicaPropose const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto reject =
        paxos_sgx::crash::CreateReplicaReject(builder, message->slot_number());
    auto msg = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_replica_reject, ticket,
        paxos_sgx::crash::BasicMessage_ReplicaReject, reject.Union());
    builder.Finish(msg);

    return paxos_sgx::crash::replicas::broadcast_message(
        builder.GetBufferPointer(), builder.GetSize());
}
}  // namespace

namespace paxos_sgx {
namespace crash {
namespace handler {

int replica_fast_get_handler(peer &p, int64_t ticket,
                             paxos_sgx::crash::FastGetArgs const *args) {
    LOG("replica fast get request [%ld]: account %ld", ticket, args->account());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_fast_get_resp_handler(
    peer &p, int64_t ticket,
    paxos_sgx::crash::ReplicaFastGetResult const *resp) {
    LOG("replica fast get response [%ld]: account %ld", ticket,
        resp->account());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_propose_handler(peer &p, int64_t ticket,
                            paxos_sgx::crash::ReplicaPropose const *message) {
    LOG("replica propose request [%ld]: slot %ld", ticket,
        message->slot_number());

    if (g_log.add_op(message->slot_number(), message->operation_args())) {
        replicas::add_accept(message->slot_number());
        return broadcast_accept(ticket, message);
    }

    return broadcast_reject(ticket, message);
}

int replica_accept_handler(peer &p, int64_t ticket,
                           paxos_sgx::crash::ReplicaAccept const *message) {
    LOG("replica accept request [%ld]: slot %ld", ticket,
        message->slot_number());

    paxos_sgx::crash::replicas::add_accept(message->slot_number());
    return 0;
}

int replica_reject_handler(peer &p, int64_t ticket,
                           paxos_sgx::crash::ReplicaReject const *message) {
    LOG("replica reject request [%ld]: slot %ld", ticket,
        message->slot_number());
    KILL("Without a view change, there should never be a rejection");
    return 0;
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
