#pragma once

#include "config.h"
#include "peer.h"

#include <fcntl.h>

namespace register_sgx {
namespace restart_rollback {

// network helpers
enum class process_res { HANDLED_MSG, NOOP, ERR };


int build_fd_sets(peer const &server, fd_set *read_fds, fd_set *write_fds,
                  fd_set *except_fds);
int build_all_fd_sets(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);

int block_until_return(size_t idx, peer &server);
int connect_to_proxy(config_node_t const &peer_node);

process_res process_peer(size_t idx, peer &p, struct timeval *timeout = nullptr);
process_res process_peers(struct timeval *timeout = nullptr);

}  // namespace restart_rollback
}  // namespace register_sgx
