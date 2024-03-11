#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "peer.h"
#include "log.h"

extern std::vector<peer> g_servers;

inline size_t quorum_size() { return 2 * ((g_servers.size() - 1) / 3) + 1; }
inline size_t commit_quorum_size() { return ((g_servers.size() - 1) / 3) + 1; }

struct FastGetContext {
    int64_t m_amount = -1;
    int64_t m_last_applied = -1;
    size_t m_reply_n = 0;

    bool add_call(int64_t amount, int64_t last_applied, int64_t last_accepted) {
        if (last_applied != last_accepted) {
            return false; // not consistent
        }

        if (m_amount == -1) {
            m_amount = amount;
            m_last_applied = last_applied;
        } else if (amount != m_amount || last_applied != m_last_applied) {
            return false;
        }

        m_reply_n += 1;

        return true;
    }

    bool ready() {
        return (m_reply_n >= quorum_size());
    }
};

struct TransferContext {
    int64_t m_amount = -1;
    bool m_success = false;
    size_t m_reply_n = 0;

    bool add_call(int64_t amount, bool success) {
        if (m_amount == -1) {
            m_amount = amount;
            m_success = success;
        } else if (amount != m_amount || success != m_success) {
            return false;
        }

        m_reply_n += 1;
        return true;
    }

    bool ready() {
        return (m_reply_n >= commit_quorum_size());
    }
};
