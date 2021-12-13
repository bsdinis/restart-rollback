#pragma once

#include <stdint.h>
#include <sys/types.h>

extern size_t read_quorum_size(size_t);
extern size_t max_quorum_size(size_t);

struct FastGetContext {
    int64_t m_amount = -1;
    int64_t m_last_applied = -1;
    size_t m_reply_n = 0;
    size_t m_suspicions = 0;
    bool   m_with_read_quorum = true;

    bool add_call(int64_t amount, int64_t last_applied, int64_t last_accepted, int64_t last_seen, bool sus) {
        if (last_applied != last_accepted) {
            return false; // not self consistent
        }

        if (m_amount == -1) {
            m_amount = amount;
            m_last_applied = last_applied;
        } else if (amount != m_amount || last_applied != m_last_applied) {
            return false; // not consistent
        }

        m_reply_n += 1;
        if (sus) {
            m_suspicions += 1;
        }
        if (last_seen > last_applied) {
            m_with_read_quorum = false;
        }

        return true;
    }

    bool ready() {
        if (m_with_read_quorum) {
            return m_reply_n >= read_quorum_size(m_suspicions);
        } else {
            return m_reply_n >= max_quorum_size(m_suspicions);
        }
    }
};
