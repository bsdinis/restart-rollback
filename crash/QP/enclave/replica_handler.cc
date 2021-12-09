#include "replica_handler.h"

#include "crash_generated.h"
#include "log.h"

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
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_accept_handler(peer &p, int64_t ticket,
                           paxos_sgx::crash::ReplicaAccept const *message) {
    LOG("replica accept request [%ld]: slot %ld", ticket,
        message->slot_number());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_reject_handler(peer &p, int64_t ticket,
                           paxos_sgx::crash::ReplicaReject const *message) {
    LOG("replica reject request [%ld]: slot %ld", ticket,
        message->slot_number());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
