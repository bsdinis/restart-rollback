#pragma once

#include <stdlib.h>
#include "peer.h"
#include <functional>

namespace register_sgx {
namespace byzantine {

extern int64_t g_call_ticket;

enum class call_type { SYNC, ASYNC, CALLBACK };

// ticket numbers have semantic.
// specifically, the remainder mod 3 indicates
// which type of call they relate to
inline int64_t gen_ticket(call_type type) {
    int const target = type == call_type::SYNC ? 0 : (type == call_type::ASYNC ? 1 : 2);
    g_call_ticket += 3 - (g_call_ticket % 3) + target;
    return g_call_ticket;
}

// protocol helpers
int64_t send_get_request(int64_t key, call_type type);
int send_writeback_request_to(int64_t ticket, int64_t key, std::array<uint8_t, 2048> const &value, int64_t timestamp, call_type type, std::vector<size_t> send_list);

int64_t send_get_timestamp_request(int64_t key, call_type type);
int send_put_request(int64_t ticket, int64_t key, std::array<uint8_t, 2048> const &value, int64_t timestamp, call_type type);

int64_t send_proxy_get_request(peer &server, int64_t key, call_type type);
int64_t send_proxy_put_request(peer &server, int64_t key, std::array<uint8_t, 2048> const &value, call_type type);

int64_t send_ping_request(peer &server, call_type type);
int64_t send_reset_request(peer &server, call_type type);


int handle_received_message(size_t idx, peer &p);
bool has_result(int64_t ticket);
}  // namespace byzantine
}  // namespace register_sgx
