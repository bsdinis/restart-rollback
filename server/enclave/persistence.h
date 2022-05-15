#pragma once

#include "policy.h"

namespace teems {

int log_accepted(size_t slot_n, int64_t key, bool policy_read,
                 ServerPolicy const &policy);

}  // namespace teems
