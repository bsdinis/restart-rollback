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
namespace crash {
namespace handler {

int handle_new_connection(int const listen_socket, std::vector<peer> &list);
int handle_client_message(peer &p);
int handle_replica_message(peer &p, size_t idx);

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
