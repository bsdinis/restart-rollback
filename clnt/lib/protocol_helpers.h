#pragma once

#include <functional>
#include <stdlib.h>

#include "peer.h"
#include "types.h"

namespace teems {

int64_t send_ping_request(peer &server, call_type type);
int64_t send_reset_request(peer &server, call_type type);


int handle_received_message(size_t idx, peer &p);
bool has_result(int64_t ticket);

}  // namespace teems
