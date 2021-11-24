#include "close_handler.h"

#include "basicQP_resp_generated.h"
#include "log.h"
#include "setup.h"

namespace epidemics {
namespace basicQP {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace basicQP
}  // namespace epidemics
