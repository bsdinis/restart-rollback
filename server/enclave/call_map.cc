#include "call_map.h"
#include "handler_helpers.h"
#include "log.h"
#include "protocol_handler.h"
#include "teems_generated.h"

namespace teems {

// smr context

int64_t SmrCallContext::ticket() const {
    if (m_parent_get_call != nullptr) {
        return m_parent_get_call->ticket();
    } else {
        return m_parent_put_call->ticket();
    }
}

int64_t SmrCallContext::key() const {
    if (m_parent_get_call != nullptr) {
        return m_parent_get_call->key();
    } else {
        return m_parent_put_call->key();
    }
}

void SmrCallContext::finish(bool success, int64_t policy_version,
                            ServerPolicy policy) {
    m_success = success;
    m_policy_version = policy_version;
    m_policy = policy;

    if (m_parent_get_call != nullptr) {
        m_parent_get_call->finished_smr(this);
    } else {
        m_parent_put_call->finished_smr(this);
    }
}

// get context

GetNextAction GetCallContext::add_get_reply(
    ssize_t peer_idx, int64_t policy_version, int64_t timestamp, bool stable,
    bool suspicious, ServerPolicy const& policy,
    std::array<uint8_t, REGISTER_SIZE> const& value) {
    if (m_get_retrying) {
        return GetNextAction::None;
    }

    if (m_n_get_replies == 0) {
        m_policy_version = policy_version;
        m_policy = policy;
    } else if (m_policy_version != policy_version) {
        LOG("policy version is not unanimous, fallback to SMR read");
        m_get_retrying = true;
        m_n_get_replies = 0;
        m_n_suspicions = 0;
        m_n_up_to_date = 0;

        m_value.fill(0);
        m_stable = false;
        m_peer_timestamps.clear();
        m_peer_stable.clear();
        m_outdated_idx.clear();
        m_unstable_idx.clear();
        return GetNextAction::FallbackPolicyRead;
    }

    m_n_get_replies += 1;

    if (suspicious) {
        m_n_suspicions += 1;
    }

    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
        m_stable = stable;
        std::copy(std::begin(value), std::end(value), std::begin(m_value));
    } else if (timestamp == m_timestamp) {
        m_stable = m_stable || stable;
    }

    if (peer_idx >= 0) {
        m_peer_timestamps[peer_idx] = timestamp;
        m_peer_stable[peer_idx] = stable;
    } else {
        m_self_timestamp = timestamp;
        m_self_stable = stable;
    }

    if (early_get_done()) {
        // stable read quorum
        finish_get_phase();
        m_get_round_done = true;
        m_call_done = true;
        return GetNextAction::ReturnToClient;
    } else if (full_get_done()) {
        finish_get_phase();

        m_get_round_done = true;
        if (m_n_up_to_date < setup::write_quorum_size()) {
            // unstable and disagreeing write quorum
            return GetNextAction::Writeback;
        } else {
            // unstable but unanimous quorum
            m_call_done = true;
            return GetNextAction::StabilizeAndReturnToClient;
        }
    }

    return GetNextAction::None;
}

GetNextAction GetCallContext::add_get_retry_reply(
    ssize_t peer_idx, int64_t timestamp, bool stable, bool suspicious,
    std::array<uint8_t, REGISTER_SIZE> const& value) {
    if (!m_get_retrying) {
        ERROR("got a retry reply without being in retry mode");
        return GetNextAction::None;
    }

    m_n_get_replies += 1;

    if (suspicious) {
        m_n_suspicions += 1;
    }

    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
        m_stable = stable;
        std::copy(std::begin(value), std::end(value), std::begin(m_value));
    } else if (timestamp == m_timestamp) {
        m_stable = m_stable || stable;
    }

    if (peer_idx >= 0) {
        m_peer_timestamps[peer_idx] = timestamp;
        m_peer_stable[peer_idx] = stable;
    } else {
        m_self_timestamp = timestamp;
        m_self_stable = stable;
    }

    if (early_get_done()) {
        // stable read quorum
        finish_get_phase();
        m_get_round_done = true;
        m_call_done = true;
        return GetNextAction::ReturnToClient;
    } else if (full_get_done()) {
        finish_get_phase();

        m_get_round_done = true;
        if (m_n_up_to_date < setup::write_quorum_size()) {
            // unstable and disagreeing write quorum
            return GetNextAction::Writeback;
        } else {
            // unstable but unanimous quorum
            m_call_done = true;
            return GetNextAction::StabilizeAndReturnToClient;
        }
    }

    return GetNextAction::None;
}

GetNextAction GetCallContext::add_writeback_reply() {
    m_n_up_to_date += 1;
    if (m_n_up_to_date >= setup::write_quorum_size()) {
        m_call_done = true;
        return GetNextAction::StabilizeAndReturnToClient;
    } else {
        return GetNextAction::None;
    }
}

void GetCallContext::finish_get_phase() {
    for (size_t idx = 0; idx < m_peer_timestamps.size(); ++idx) {
        auto const& timestamp = m_peer_timestamps[idx];
        if (timestamp < m_timestamp) {
            m_outdated_idx.emplace_back(idx);
        } else if (timestamp == m_timestamp) {
            m_n_up_to_date += 1;
            if (!m_peer_stable[idx]) {
                m_unstable_idx.emplace_back(idx);
            }
        }
    }

    if (!self_outdated()) {
        m_n_up_to_date += 1;
    }
}

int GetCallContext::finished_smr(SmrCallContext const* smr_context) {
    if (smr_context->success()) {
        m_policy_version = smr_context->policy_version();
        m_policy = smr_context->policy();
    } else {
        m_policy_version = 1;
        m_policy = ServerPolicy(0xff, m_client_id, 0);
    }

    return handler::get_retry_get(*this);
}

// put context
PutNextAction PutCallContext::add_get_reply(int64_t policy_version,
                                            int64_t timestamp, bool suspicious,
                                            ServerPolicy const& policy) {
    if (m_get_retrying) {
        return PutNextAction::None;
    }
    if (m_n_get_replies == 0) {
        m_policy_version = policy_version;
        m_policy = policy;
    } else if (m_policy_version != policy_version) {
        LOG("policy version is not unanimous, fallback to SMR read");
        m_get_retrying = true;
        m_n_get_replies = 0;
        m_n_suspicions = 0;
        m_n_put_replies = 0;
        return PutNextAction::FallbackPolicyRead;
    }

    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
    }

    if (suspicious) {
        m_n_suspicions += 1;
    }
    m_n_get_replies += 1;

    if (m_n_get_replies >= setup::read_quorum_size(m_n_suspicions)) {
        m_get_round_done = true;
        return PutNextAction::DoWrite;
    }

    return PutNextAction::None;
}

// put context
PutNextAction PutCallContext::add_get_retry_reply(int64_t timestamp,
                                                  bool suspicious) {
    if (!m_get_retrying) {
        ERROR("got a retry reply without being in retry mode");
        return PutNextAction::None;
    }
    if (timestamp > m_timestamp) {
        m_timestamp = timestamp;
    }

    if (suspicious) {
        m_n_suspicions += 1;
    }
    m_n_get_replies += 1;

    if (m_n_get_replies >= setup::read_quorum_size(m_n_suspicions)) {
        m_get_round_done = true;
        return PutNextAction::DoWrite;
    }

    return PutNextAction::None;
}

PutNextAction PutCallContext::add_put_reply() {
    m_n_put_replies += 1;
    if (m_n_put_replies >= setup::write_quorum_size()) {
        m_call_done = true;
        return PutNextAction::ReturnToClient;
    }

    return PutNextAction::None;
}

int PutCallContext::finished_smr(SmrCallContext const* smr_context) {
    if (smr_context->success()) {
        m_policy_version = smr_context->policy_version();
        m_policy = smr_context->policy();
    } else {
        m_policy_version = 1;
        m_policy = ServerPolicy(0xff, m_client_id, 0);
    }

    return handler::put_retry_get(*this);
}

// call map

SmrCallContext* CallMap::add_smr_call(GetCallContext* parent_call,
                                      size_t slot_n) {
    return &m_smr_map.emplace(slot_n, SmrCallContext(parent_call, slot_n))
                .first->second;
}
SmrCallContext* CallMap::add_smr_call(PutCallContext* parent_call,
                                      size_t slot_n) {
    return &m_smr_map.emplace(slot_n, SmrCallContext(parent_call, slot_n))
                .first->second;
}

GetCallContext* CallMap::add_get_call(peer* client, int32_t client_id,
                                      int64_t ticket, int64_t key) {
    return &m_get_map
                .emplace(ticket, GetCallContext(client, client_id, ticket, key))
                .first->second;
}
PutCallContext* CallMap::add_put_call(peer* client, int32_t client_id,
                                      int64_t ticket, int64_t key,
                                      Value const* value) {
    return &m_put_map
                .emplace(ticket,
                         PutCallContext(client, client_id, ticket, key, value))
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

void CallMap::resolve_smr_call(size_t slot_n, bool success,
                               int64_t policy_version, ServerPolicy policy) {
    auto smr_it = m_smr_map.find(slot_n);
    if (smr_it != std::end(m_smr_map)) {
        smr_it->second.finish(success, policy_version, policy);
        m_smr_map.erase(smr_it);
        return;
    }

    ERROR("failed to find call for slot %zu", slot_n);
}

}  // namespace teems
