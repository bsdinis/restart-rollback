#include "close_handler.h"

#include "log.h"
#include "restart_rollback_generated.h"
#include "setup.h"

namespace register_sgx {
namespace restart_rollback {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
