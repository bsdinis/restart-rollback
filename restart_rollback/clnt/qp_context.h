#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <array>
#include <vector>

#include "log.h"
#include "qp_config.h"

namespace register_sgx{
namespace restart_rollback {

extern int32_t g_client_id;

class GetContext {
    private:
    int64_t m_key;
    int64_t m_max_timestamp = -1;
    std::vector<int64_t> m_timestamps;
    std::vector<size_t> m_outdated_idx;

    size_t m_n_get_replies = 0;
    size_t m_n_up_to_date = 0;

    std::array<uint8_t, REGISTER_SIZE> m_value;
    bool m_success = true;

    public:
    GetContext(int64_t key, size_t n_servers) : m_key(key), m_timestamps(n_servers, -1), m_outdated_idx() {
        m_outdated_idx.reserve(n_servers);
    }

    bool add_get_resp(size_t peer_idx, std::array<uint8_t, REGISTER_SIZE> const& value, int64_t timestamp) {
        m_timestamps[peer_idx] = timestamp;
        m_n_get_replies += 1;

        if (timestamp > m_max_timestamp) {
            m_max_timestamp = timestamp;
            std::copy(std::cbegin(value), std::cend(value), std::begin(m_value));
        }

        return true;
    }

    bool finished_get_phase(size_t quorum_size) const {
        return (m_n_get_replies >= quorum_size);
    }

    void finish_get_phase() {
        for (size_t idx = 0; idx < m_timestamps.size(); ++idx) {
            auto const & timestamp = m_timestamps[idx];
            if (timestamp != -1 && timestamp < m_max_timestamp) {
                m_outdated_idx.emplace_back(idx);
            } else if (timestamp == m_max_timestamp) {
                m_n_up_to_date += 1;
            }
        }
    }

    bool is_unanimous() const {
        return m_outdated_idx.size() == 0;
    }

    bool add_put_resp() {
        m_n_up_to_date += 1;
        return true;
    }

    bool finished_writeback(size_t quorum_size) const {
        return (m_n_up_to_date >= quorum_size);
    }

    inline int64_t key() const { return m_key; }
    inline int64_t timestamp() const { return m_max_timestamp; }
    inline bool success() const { return m_success; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const { return m_value; }
    inline std::vector<size_t> const& out_of_date_server_idx() const { return m_outdated_idx; }
};

class PutContext {
    private:
    int64_t m_key;
    int64_t m_max_timestamp = -1;

    size_t m_n_get_replies = 0;
    size_t m_n_put_replies = 0;
    bool m_finished_get_phase = false;

    std::array<uint8_t, REGISTER_SIZE> m_value;
    bool m_success = true;

    public:
    PutContext(int64_t key, std::array<uint8_t, REGISTER_SIZE> value) : m_key(key), m_value(value) { }

    bool add_get_resp(int64_t timestamp) {
        if (timestamp > m_max_timestamp) {
            m_max_timestamp = timestamp;
        }
        m_n_get_replies += 1;

        return true;
    }

    bool finished_get_phase(size_t quorum_size) const {
        return (m_n_get_replies >= quorum_size);
    }

    void finish_get_phase() {
        m_finished_get_phase = true;
    }

    bool started_put_phase() const {
        return m_finished_get_phase;
    }

    bool add_put_resp() {
        m_n_put_replies += 1;
        return true;
    }

    bool finished_put_phase(size_t quorum_size) const {
        return (m_n_put_replies >= quorum_size);
    }

    inline int64_t key() const { return m_key; }
    inline int64_t next_timestamp() const {
        uint64_t seqno = (m_max_timestamp < 0) ? 1 : (static_cast<uint64_t>(m_max_timestamp) >> 32) + 1;
        return static_cast<int64_t> ( (seqno << 32) | static_cast<uint64_t> (g_client_id));
    }
    inline bool success() const { return m_success; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const { return m_value; }
};

} // namespace restart_rollback
} // namespace register_sgx
