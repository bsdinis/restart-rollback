/***
 * network_wrapper.c
 */

#include "network_wrappers.h"
#include "log.h"

#include <ifaddrs.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

namespace paxos_sgx {
namespace crash {
namespace __utils {

int net_start_listen_socket(char const *server_addr, uint16_t server_port) {
    int listen_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_socket < 0) {
        std::perror("socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) != 0) {
        std::perror("setsockopt");
        return -1;
    }

    struct sockaddr_in my_addr;
    std::memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(server_addr);
    my_addr.sin_port = htons(server_port);

    if (bind(listen_socket, (struct sockaddr *)&my_addr,
             sizeof(struct sockaddr)) != 0) {
        if (errno == EADDRINUSE) {
            ERROR("port %d already in use", server_port);
        } else {
            std::perror("bind");
        }

        return -1;
    }

    // start accept client connections
    if (listen(listen_socket, 40) != 0) {
        std::perror("listen");
        return -1;
    }
    FINE("Accepting connections on port %d", server_port);

    return listen_socket;
}

char const *net_get_my_ipv4_addr(void) {
    static std::string addr;
    if (addr.length() > 0) return addr.c_str();

    struct ifaddrs *addrs;
    getifaddrs(&addrs);
    struct ifaddrs *node = addrs;

    while (node) {
        if (node->ifa_addr && node->ifa_addr->sa_family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *this_address =
                (struct sockaddr_in *)node->ifa_addr;
            inet_ntop(node->ifa_addr->sa_family, &(this_address->sin_addr), ip,
                      INET_ADDRSTRLEN);
            addr = ip;

            if (node->ifa_name[0] == 'w') {  // hack!!!! FIXME
                break;
            }
        }

        node = node->ifa_next;
    }

    freeifaddrs(addrs);
    return addr.c_str();
}

}  // namespace __utils
}  // namespace crash
}  // namespace paxos_sgx
