#include "close_handler.h"

#include "byzantine_generated.h"
#include "log.h"
#include "setup.h"

namespace register_sgx {
namespace byzantine {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
