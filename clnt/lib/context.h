#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <array>
#include <vector>

#include "log.h"
#include "teems_config.h"

namespace teems{

extern int32_t g_client_id;

namespace {
enum class ProtocolPhase {
    Read, Write
};
} // anonymous namespace

class GetContext {
    private:
    int64_t m_key;
    int64_t m_max_timestamp = -1;
    bool m_stable;
    ProtocolPhase m_phase = ProtocolPhase::Read;

    std::vector<int64_t> m_timestamps;
    std::vector<bool> m_unstables;
    std::vector<size_t> m_outdated_idx;
    std::vector<size_t> m_unstable_idx;

    size_t m_n_suspicions = 0;
    size_t m_n_get_replies = 0;
    size_t m_n_up_to_date = 0;

    std::array<uint8_t, REGISTER_SIZE> m_value;
    bool m_success = true;

    public:
    GetContext(int64_t key, size_t n_servers) : m_key(key), m_timestamps(n_servers, -1), m_unstables(n_servers, false), m_outdated_idx() {
        m_outdated_idx.reserve(n_servers);
    }

    bool add_get_resp(size_t peer_idx, std::array<uint8_t, REGISTER_SIZE> const& value, int64_t timestamp, bool stable, bool suspicious) {
        m_timestamps[peer_idx] = timestamp;
        m_unstables[peer_idx] = !stable;

        m_n_get_replies += 1;
        if (suspicious) {
            m_n_suspicions += 1;
        }

        if (timestamp > m_max_timestamp) {
            m_max_timestamp = timestamp;
            m_stable = stable;
            std::copy(std::cbegin(value), std::cend(value), std::begin(m_value));
        } else if (timestamp == m_max_timestamp) {
            m_stable = m_stable || stable;
        }

        return true;
    }

    bool finished_early_get_phase(size_t read_quorum_size) const {
        return m_n_get_replies >= read_quorum_size;
    }
    bool finished_get_phase(size_t write_quorum_size) const {
        return m_n_get_replies >= write_quorum_size;
    }

    void finish_get_phase() {
        m_phase = ProtocolPhase::Write;
        for (size_t idx = 0; idx < m_timestamps.size(); ++idx) {
            auto const & timestamp = m_timestamps[idx];
            if (timestamp != -1 && timestamp < m_max_timestamp) {
                m_outdated_idx.emplace_back(idx);
            } else if (timestamp == m_max_timestamp) {
                m_n_up_to_date += 1;
                if (m_unstables[idx]) {
                    m_unstable_idx.emplace_back(idx);
                }
            }
        }
    }

    bool is_stable() const {
        return m_stable;
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

    inline bool in_read_phase() const {
        return m_phase == ProtocolPhase::Read;
    }
    inline bool in_write_phase() const {
        return m_phase == ProtocolPhase::Write;
    }

    inline int64_t key() const { return m_key; }
    inline int64_t timestamp() const { return m_max_timestamp; }
    inline size_t n_suspicions() const { return m_n_suspicions; }
    inline bool success() const { return m_success; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const { return m_value; }
    inline std::vector<size_t> const& out_of_date_server_idx() const { return m_outdated_idx; }
    inline std::vector<size_t> const& unstable_server_idx() const { return m_unstable_idx; }
    inline std::vector<size_t> unstable_or_outdated_server_idx() const {
        std::vector<size_t> res(m_unstable_idx.size() + m_outdated_idx.size());
        std::copy(std::cbegin(m_unstable_idx), std::cend(m_unstable_idx), std::back_inserter(res));
        std::copy(std::cbegin(m_outdated_idx), std::cend(m_outdated_idx), std::back_inserter(res));
        return res;
    }
};

class PutContext {
    private:
    int64_t m_key;
    int64_t m_max_timestamp = -1;

    size_t m_n_suspicions = 0;
    size_t m_n_get_replies = 0;
    size_t m_n_put_replies = 0;
    bool m_finished_get_phase = false;

    std::array<uint8_t, REGISTER_SIZE> m_value;
    bool m_success = true;

    public:
    PutContext(int64_t key, std::array<uint8_t, REGISTER_SIZE> value) : m_key(key), m_value(value) { }

    bool add_get_resp(int64_t timestamp, bool suspicious) {
        if (suspicious) {
            m_n_suspicions += 1;
        }
        if (timestamp > m_max_timestamp) {
            m_max_timestamp = timestamp;
        }
        m_n_get_replies += 1;

        return true;
    }

    bool finished_get_phase(size_t read_quorum_size) const {
        return (m_n_get_replies >= read_quorum_size);
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

    bool finished_put_phase(size_t write_quorum_size) const {
        return (m_n_put_replies >= write_quorum_size);
    }

    inline int64_t key() const { return m_key; }
    inline size_t n_suspicions() const { return m_n_suspicions; }
    inline int64_t next_timestamp() const {
        uint64_t seqno = (m_max_timestamp < 0) ? 1 : (static_cast<uint64_t>(m_max_timestamp) >> 32) + 1;
        return static_cast<int64_t> ( (seqno << 32) | static_cast<uint64_t> (g_client_id));
    }
    inline bool success() const { return m_success; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const { return m_value; }
};

} // namespace teems
