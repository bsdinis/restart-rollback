/**
 * handlers.h
 *
 * generic handlers
 */
#pragma once

#include <stdint.h>
#include <vector>
#include "peer.h"

namespace register_sgx {
namespace restart_rollback {
namespace handler {

ssize_t handle_new_connection(int const listen_socket, std::vector<peer> &list);
int handle_client_message(peer &p);
int handle_replica_message(peer &p, size_t idx);
int send_greeting(peer &p);

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
