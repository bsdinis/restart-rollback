#include "context.h"

#include "async.h"
#include "log.h"

#include <unordered_map>

namespace teems {
//================================
// GLOBALS
//================================

::std::unordered_map<int64_t, get_call_ctx> g_get_calls;
::std::unordered_map<int64_t, put_call_ctx> g_put_calls;

int add_get_call(int64_t metadata_ticket, int64_t key) {
    auto res = g_get_calls.emplace(get_supercall_ticket(metadata_ticket),
                                   get_call_ctx(metadata_ticket, key));
    return std::get<1>(res) ? 0 : -1;
}

int add_put_call(int64_t untrusted_ticket, int64_t key, Metadata&& metadata) {
    auto res = g_put_calls.emplace(
        get_supercall_ticket(untrusted_ticket),
        put_call_ctx(untrusted_ticket, key, std::move(metadata)));
    return std::get<1>(res) ? 0 : -1;
}

int rem_get_call(int64_t super_ticket) {
    auto it = g_get_calls.find(super_ticket);
    if (it == g_get_calls.end()) {
        return -1;
    }

    g_get_calls.erase(it);
    return 0;
}
int rem_put_call(int64_t super_ticket) {
    auto it = g_put_calls.find(super_ticket);
    if (it == g_put_calls.end()) {
        return -1;
    }

    g_put_calls.erase(it);
    return 0;
}

get_call_ctx* get_get_call_ctx(int64_t teems_ticket) {
    auto it = g_get_calls.find(teems_ticket);
    if (it == g_get_calls.end()) {
        return nullptr;
    }

    return &it->second;
}
put_call_ctx* get_put_call_ctx(int64_t teems_ticket) {
    auto it = g_put_calls.find(teems_ticket);
    if (it == g_put_calls.end()) {
        return nullptr;
    }

    return &it->second;
}

int get_call_tickets(int64_t teems_ticket, int64_t* metadata_ticket,
                     int64_t* untrusted_ticket) {
    auto get_ctx = get_get_call_ctx(teems_ticket);
    auto put_ctx = get_put_call_ctx(teems_ticket);

    if (get_ctx != nullptr) {
        *metadata_ticket = get_ctx->metadata_ticket();
        *untrusted_ticket = get_ctx->untrusted_ticket();
    } else if (put_ctx != nullptr) {
        *metadata_ticket = put_ctx->metadata_ticket();
        *untrusted_ticket = put_ctx->untrusted_ticket();
    } else {
        *metadata_ticket = -1;
        *untrusted_ticket = -1;
        return -1;
    }

    return 0;
}

}  // namespace teems
