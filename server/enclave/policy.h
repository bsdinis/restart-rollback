#pragma once

#include <cstdint>
#include "teems_generated.h"

namespace teems {

class ServerPolicy {
   public:
    ServerPolicy() = default;
    ServerPolicy(uint8_t code, int32_t owner_id, int64_t valid_from)
        : m_code(code), m_owner_id(owner_id), m_valid_from(valid_from) {}

    ServerPolicy(Policy const *policy)
        : m_code(policy->policy_code()),
          m_owner_id(policy->owner_id()),
          m_valid_from(policy->valid_from()) {}

    bool can_get(int32_t id) const;
    bool can_put(int32_t id) const;
    bool can_change_policy(int32_t id) const;
    Policy to_flatbuffers() const;

    inline uint8_t code() const { return m_code; }
    inline int32_t owner_id() const { return m_owner_id; }
    inline int64_t valid_from() const { return m_valid_from; }

   private:
    uint8_t m_code = 0xff;
    int32_t m_owner_id = 0;
    int64_t m_valid_from = 0;
};

}  // namespace teems
