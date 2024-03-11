/**
 * fd_helper.h
 *
 * helper struct and func to build fd sets
 */
#ifndef __FD_HELPER_HEADER__
#define __FD_HELPER_HEADER__

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "log.h"
#include "peer.h"
#include "user_types.h"

namespace register_sgx {
namespace byzantine {
namespace net {

// returns activity
int select_list(int client_listen_socket, int replica_listen_socket,
                std::vector<peer> const &client_list,
                std::vector<peer> const &replica_list);

}  // namespace net
}  // namespace byzantine
}  // namespace register_sgx

#endif  //__FD_HELPER_HEADER__
