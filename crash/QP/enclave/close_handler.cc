#include "close_handler.h"

#include "crash_generated.h"
#include "log.h"
#include "setup.h"

namespace register_sgx {
namespace crash {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
