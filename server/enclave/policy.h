#pragma once

#include <cstdint>
#include "teems_generated.h"

namespace teems {

class ServerPolicy {
   public:
    ServerPolicy() = default;
    ServerPolicy(uint8_t policy_code, int32_t owner_id, int64_t valid_from)
        : m_policy_code(policy_code),
          m_owner_id(owner_id),
          m_valid_from(valid_from) {}

    ServerPolicy(Policy const *policy)
        : m_policy_code(policy->policy_code()),
          m_owner_id(policy->owner_id()),
          m_valid_from(policy->valid_from()) {}

    bool can_get(int32_t id) const;
    bool can_put(int32_t id) const;
    bool can_change_policy(int32_t id) const;
    Policy to_flatbuffers() const;

   private:
    uint8_t m_policy_code = 0;
    int32_t m_owner_id = 0;
    int64_t m_valid_from = 0;
};

}  // namespace teems
