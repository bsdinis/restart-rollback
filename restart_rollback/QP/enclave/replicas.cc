#include "replicas.h"
#include "call_map.h"
#include "log.h"
#include "op_log.h"
#include "peer.h"
#include "setup.h"
#include "state_machine.h"

extern std::vector<peer> g_replica_list;
extern paxos_sgx::restart_rollback::CallMap g_call_map;
extern paxos_sgx::restart_rollback::OpLog g_log;
extern paxos_sgx::restart_rollback::StateMachine g_state_machine;

namespace paxos_sgx {
namespace restart_rollback {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size) {
    int ret = 0;
    for (auto &replica : g_replica_list) {
        ret |= replica.append((uint8_t *)&size, sizeof(size_t));
        ret |= replica.append(message, size);
    };

    return ret;
}

void execute(size_t slot_number) {
    int64_t account = 0;
    int64_t amount = 0;
    bool success = g_state_machine.execute(g_log.get_operation(slot_number),
                                           account, amount);
    g_log.executed(slot_number);

    if (paxos_sgx::restart_rollback::setup::is_leader()) {
        g_call_map.resolve_call(slot_number, account, amount, success);
    }
}

void add_accept(size_t slot_number, bool sus) {
    g_log.add_accept(slot_number, sus);
    while (g_log.can_execute(slot_number)) {
        execute(slot_number);
        slot_number += 1;
    }
}

}  // namespace replicas
}  // namespace restart_rollback
}  // namespace paxos_sgx
