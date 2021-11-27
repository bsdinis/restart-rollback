/**
 * handlers.h
 *
 * generic handlers
 */
#ifndef __HANDLERS_HEADER__
#define __HANDLERS_HEADER__

#include <stdint.h>
#include <vector>
#include "peer.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int handle_new_connection(int const listen_socket, std::vector<peer> &list);

int handle_client_message(peer &p);

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx

#endif  // __HANDLERS_HEADER__
