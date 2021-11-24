#include "fd_helper.h"
#include "log.h"

namespace epidemics {
namespace basicQP {
namespace net {

// returns effective size
int select_list(int listen_socket, std::vector<peer> const &clnt_list) {
    if (listen_socket == -1)
        KILL("cannot select listen socket -1");

    std::vector<selected_t> list;
    selected_t s = {listen_socket, 0};
    list.push_back(s);
    for (auto &p : clnt_list) {
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
}  // namespace basicQP
}  // namespace epidemics
