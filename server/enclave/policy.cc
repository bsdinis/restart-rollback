#include "policy.h"

namespace teems {

// ServerPolicy semantics
//
// 8 bit code
//
// bit 7: x
// bit 6: x
// bit 5: x
// bit 4: owner can write?
// bit 3: owner can read?
// bit 2: world can change policy?
// bit 1: world can write?
// bit 0: world can read?
//
// (owner can always change policy)

bool ServerPolicy::can_get(int32_t id) const {
    if (id == m_owner_id) {
        return (m_policy_code & (0b1 << 3)) != 0;
    }

    return (m_policy_code & (0b1 << 0)) != 0;
}

bool ServerPolicy::can_put(int32_t id) const {
    if (id == m_owner_id) {
        return (m_policy_code & (0b1 << 4)) != 0;
    }

    return (m_policy_code & (0b1 << 1)) != 0;
}

bool ServerPolicy::can_change_policy(int32_t id) const {
    if (id == m_owner_id) {
        return true;
    }

    return (m_policy_code & (0b1 << 2)) != 0;
}

Policy ServerPolicy::to_flatbuffers() const {
    return Policy(m_policy_code, m_owner_id, m_valid_from);
}

}  // namespace teems
