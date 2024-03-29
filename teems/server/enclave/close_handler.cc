#include "close_handler.h"

#include "log.h"
#include "setup.h"
#include "teems_generated.h"

namespace teems {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace teems
