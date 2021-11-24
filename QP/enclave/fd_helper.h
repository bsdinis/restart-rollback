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

namespace epidemics {
namespace basicQP {
namespace net {

// returns activity
int select_list(int listen_socket, std::vector<peer> const &clnt_list);

}  // namespace net
}  // namespace basicQP
}  // namespace epidemics

#endif  //__FD_HELPER_HEADER__
