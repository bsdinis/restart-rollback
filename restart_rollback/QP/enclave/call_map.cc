#include "call_map.h"
#include "handler_helpers.h"
#include "log.h"
#include "restart_rollback_generated.h"

namespace register_sgx {
namespace restart_rollback {

// get context

void GetCallContext::add_get_reply(
    int64_t peer_idx, int64_t timestamp,
    std::array<uint8_t, REGISTER_SIZE> const& value) {
    m_n_get_replies += 1;
    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
        std::copy(std::begin(value), std::end(value), std::begin(m_value));
    }

    if (peer_idx >= 0) {
        m_peer_timestamps[peer_idx] = timestamp;
    } else {
        m_self_timestamp = timestamp;
    }
}
void GetCallContext::add_writeback_reply() { m_n_up_to_date += 1; }

void GetCallContext::finish_get_phase() {
    for (size_t idx = 0; idx < m_peer_timestamps.size(); ++idx) {
        auto const& timestamp = m_peer_timestamps[idx];
        if (timestamp < m_timestamp) {
            m_outdated_idx.emplace_back(idx);
        } else if (timestamp == m_timestamp) {
            m_n_up_to_date += 1;
        }
    }

    if (!self_outdated()) {
        m_n_up_to_date += 1;
    }
}

// put context
void PutCallContext::add_get_reply(int64_t timestamp) {
    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
    }
    m_n_get_replies += 1;
}
void PutCallContext::add_put_reply() { m_n_put_replies += 1; }

// call map

GetCallContext* CallMap::add_get_call(peer* client, int64_t ticket,
                                      int64_t key) {
    return &m_get_map.emplace(ticket, GetCallContext(client, ticket, key))
                .first->second;
}
PutCallContext* CallMap::add_put_call(peer* client, int64_t ticket, int64_t key,
                                      int32_t client_id, Value const* value) {
    return &m_put_map
                .emplace(ticket,
                         PutCallContext(client, ticket, key, client_id, value))
                .first->second;
}

GetCallContext* CallMap::get_get_ctx(int64_t ticket) {
    auto get_it = m_get_map.find(ticket);
    if (get_it == std::end(m_get_map)) {
        return nullptr;
    }

    return &get_it->second;
}

PutCallContext* CallMap::get_put_ctx(int64_t ticket) {
    auto put_it = m_put_map.find(ticket);
    if (put_it == std::end(m_put_map)) {
        return nullptr;
    }

    return &put_it->second;
}

void CallMap::resolve_call(int64_t ticket) {
    auto get_it = m_get_map.find(ticket);
    if (get_it != std::end(m_get_map)) {
        m_get_map.erase(get_it);
        return;
    }

    auto put_it = m_put_map.find(ticket);
    if (put_it != std::end(m_put_map)) {
        m_put_map.erase(put_it);
        return;
    }

    ERROR("failed to find call for ticket %zd", ticket);
    return;
}

}  // namespace restart_rollback
}  // namespace register_sgx
