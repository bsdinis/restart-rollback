#include "close_handler.h"

#include "pbft_resp_generated.h"
#include "log.h"
#include "setup.h"

namespace paxos_sgx {
namespace pbft {
namespace handler {

int close_handler(peer &p, int64_t ticket) {
    LOG("close request [%ld]", ticket);
    setup::close();
    return 0;
}

}  // namespace handler
}  // namespace pbft
}  // namespace paxos_sgx
