#include "close_handler.h"

#include "restart_rollback_resp_generated.h"
#include "log.h"
#include "setup.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
