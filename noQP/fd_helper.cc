#include "fd_helper.h"
#include "log.h"

#include <algorithm>

namespace epidemics {
namespace basicQP {
namespace net {

// returns effective size
int select_list(int listen_socket, std::vector<peer> const &clnt_list,
                fd_set *read_fds, fd_set *write_fds, fd_set *except_fds) {
    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    FD_ZERO(except_fds);

    int high_sock = listen_socket;
    FD_SET(listen_socket, read_fds);
    FD_SET(listen_socket, except_fds);

    for (auto &p : clnt_list) {
        if (!p.connected()) continue;
        FD_SET(p.sock(), read_fds);
        FD_SET(p.sock(), except_fds);
        if (p.want_write()) FD_SET(p.sock(), write_fds);
        high_sock = std::max(high_sock, p.sock());
    }

    return select(high_sock + 1, read_fds, write_fds, except_fds, NULL);
}

}  // namespace net
}  // namespace basicQP
}  // namespace epidemics
