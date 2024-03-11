#pragma once

#include <cstdint>

#include "peer.h"
#include "teems_generated.h"

namespace teems {
namespace handler {

int smr_propose_handler(peer &p, int64_t ticket, SmrPropose const *message);
int smr_accept_handler(peer &p, int64_t ticket, SmrAccept const *message);
int smr_reject_handler(peer &p, int64_t ticket, SmrReject const *message);

}  // namespace handler
}  // namespace teems
