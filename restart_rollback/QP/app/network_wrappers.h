/***
 * network_wrapper.h
 */
#pragma once

#include <arpa/inet.h>
#include <sys/types.h>

namespace paxos_sgx {
namespace restart_rollback {
namespace __utils {

int net_start_listen_socket(char const *server_addr, uint16_t server_port);
char const *net_get_my_ipv4_addr(
    void);  // static storage, does not transfer ownership

}  // namespace __utils
}  // namespace restart_rollback
}  // namespace paxos_sgx
