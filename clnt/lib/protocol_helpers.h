#pragma once

#include <functional>
#include <stdlib.h>
#include <vector>

#include "async.h"
#include "metadata.h"
#include "peer.h"

namespace teems {

int64_t send_ping_request(peer &server, call_type type);
int64_t send_reset_request(peer &server, call_type type);

int teems_handle_metadata_get(int64_t ticket, int64_t key, Metadata value, int64_t policy_version, int64_t timestamp, bool success);
int teems_handle_metadata_put(int64_t ticket, int64_t policy_version, int64_t timestamp, bool success);

int teems_handle_untrusted_get(int64_t ticket, bool success, std::vector<uint8_t> && value);
int teems_handle_untrusted_put(int64_t ticket, bool success);

int handle_received_message(size_t idx, peer &p);

}  // namespace teems
