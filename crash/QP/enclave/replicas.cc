#include "replicas.h"
#include "log.h"
#include "peer.h"
#include "setup.h"

extern std::vector<peer> g_replica_list;

namespace register_sgx {
namespace crash {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size) {
    int ret = 0;
    for (auto &replica : g_replica_list) {
        ret |= replica.append((uint8_t *)&size, sizeof(size_t));
        ret |= replica.append(message, size);
    };

    return ret;
}

}  // namespace replicas
}  // namespace crash
}  // namespace register_sgx
