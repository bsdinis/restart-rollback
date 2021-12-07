#include "replica_handler.h"

#include "crash_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int replica_fast_get_handler(peer &p, int64_t ticket,
                             paxos_sgx::crash::ReplicaFastGetArgs const *args) {
    LOG("replica fast get request [%ld]: account %ld", ticket, args->account());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_propose_handler(peer &p, int64_t ticket,
                            paxos_sgx::crash::ReplicaProposeArgs const *args) {
    LOG("replica propose request [%ld]: slot %ld", ticket, args->slot_number());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

int replica_accept_handler(peer &p, int64_t ticket,
                           paxos_sgx::crash::ReplicaAcceptArgs const *args) {
    LOG("replica accept request [%ld]: slot %ld", ticket, args->slot_number());
    ERROR("UNIMPLEMENTED!");
    return 0;
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
