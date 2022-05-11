#include "fd_helper.h"
#include "log.h"

namespace teems {
namespace net {

// returns effective size
int select_list(int client_listen_socket, int replica_listen_socket,
                std::vector<peer> const &client_list,
                std::vector<peer> const &replica_list) {
    if (client_listen_socket == -1) {
        KILL("cannot select client listen socket -1");
    }
    if (replica_listen_socket == -1) {
        KILL("cannot select replica listen socket -1");
    }

    std::vector<selected_t> list;
    selected_t client_s = {client_listen_socket, 0};
    selected_t replica_s = {replica_listen_socket, 0};
    list.push_back(client_s);
    list.push_back(replica_s);

    for (auto &p : client_list) {
        if (!p.connected()) continue;
        selected_t s = {p.sock(), p.want_write() ? 1 : 0};
        list.push_back(s);
    }
    for (auto &p : replica_list) {
        if (!p.connected()) continue;
        selected_t s = {p.sock(), p.want_write() ? 1 : 0};
        list.push_back(s);
    }

    int activity = 0;
    sgx_status_t status =
        ocall_select_list(&activity, list.data(), list.size());
    if (status != SGX_SUCCESS) {
        ERROR("sgx select list failed");
        return -1;
    }
    return activity;
}

}  // namespace net
}  // namespace teems
