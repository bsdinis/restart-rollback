#include "replicas.h"

#include "call_map.h"
#include "kv_store.h"
#include "log.h"
#include "op_log.h"
#include "peer.h"
#include "setup.h"

extern std::vector<peer> g_replica_list;
extern teems::CallMap g_call_map;
extern teems::KeyValueStore g_kv_store;
extern teems::OpLog g_log;

namespace teems {
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
    Operation const *operation = g_log.get_operation(slot_number);
    bool success = false;
    int64_t policy_version = -1;
    ServerPolicy policy;
    if (operation->m_policy_read) {
        success =
            g_kv_store.get_policy(operation->m_key, &policy, &policy_version);
    } else {
        success = g_kv_store.change_policy(
            operation->m_key, operation->m_policy, &policy_version);
        policy = operation->m_policy;
    }

    g_log.executed(slot_number);

    if (setup::is_leader()) {
        g_call_map.resolve_smr_call(slot_number, success, policy_version,
                                    policy);
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
}  // namespace teems
