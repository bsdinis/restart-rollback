/**
 * call_map.h
 *
 * The map of outstanding calls
 */

#pragma once
#include <algorithm>
#include <array>
#include <unordered_map>

#include "peer.h"
#include "policy.h"
#include "setup.h"
#include "teems_generated.h"
#include "user_types.h"

extern std::vector<peer> g_replica_list;

namespace teems {

enum class GetNextAction {
    None,
    FallbackPolicyRead,
    Writeback,
    StabilizeAndReturnToClient,
    ReturnToClient,
};

class GetCallContext {
   public:
    GetCallContext(peer* client, int32_t client_id, int64_t ticket, int64_t key)
        : m_client(client),
          m_client_id(client_id),
          m_ticket(ticket),
          m_key(key),
          m_peer_timestamps(std::vector<int64_t>(g_replica_list.size(), -1)),
          m_peer_stable(std::vector<bool>(g_replica_list.size(), false)) {}

    // add a new reply
    GetNextAction add_get_reply(
        ssize_t peer_idx, int64_t policy_version, int64_t timestamp,
        bool stable, bool suspicious, ServerPolicy const& policy,
        std::array<uint8_t, REGISTER_SIZE> const& value);
    GetNextAction add_writeback_reply();

    inline bool get_done() const { return m_get_round_done; }
    inline bool call_done() const { return m_call_done; }

    inline bool self_outdated() const { return m_self_timestamp < m_timestamp; }
    inline bool self_unstable() const { return !m_self_stable; }

    // getters
    inline int64_t ticket() const { return m_ticket; }
    inline int64_t key() const { return m_key; }
    inline int64_t policy_version() const { return m_policy_version; }
    inline int64_t timestamp() const { return m_timestamp; }
    inline ServerPolicy const& policy() const { return m_policy; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const& {
        return m_value;
    }

    inline std::vector<size_t> const& writeback_indices() const& {
        return m_outdated_idx;
    }
    inline std::vector<size_t> const& unstable_indices() const& {
        return m_unstable_idx;
    }
    inline std::vector<size_t> all_stabilize_indices() const& {
        std::vector<size_t> result(m_outdated_idx.size() +
                                   m_unstable_idx.size());
        std::copy(std::begin(m_unstable_idx), std::end(m_unstable_idx),
                  std::back_inserter(result));
        std::copy(std::begin(m_outdated_idx), std::end(m_outdated_idx),
                  std::back_inserter(result));
        return result;
    }
    inline peer* client() const { return m_client; }
    inline int32_t client_id() const { return m_client_id; }

   private:
    void finish_get_phase();

    inline bool early_get_done() const {
        return m_stable &&
               m_n_get_replies >= setup::read_quorum_size(m_n_suspicions);
    }
    inline bool full_get_done() const {
        return m_n_get_replies >= setup::write_quorum_size();
    }

    inline bool is_stable() const { return m_stable; }

    peer* m_client = nullptr;
    int32_t m_client_id = -1;

    size_t m_n_get_replies = 0;
    size_t m_n_suspicions = 0;
    size_t m_n_up_to_date = 0;

    int64_t m_self_policy_version = -1;
    int64_t m_self_timestamp = -1;
    bool m_self_stable = -1;

    int64_t m_key = -1;
    int64_t m_ticket = -1;
    int64_t m_policy_version = -1;
    int64_t m_timestamp = -1;
    bool m_stable = -1;
    std::vector<int64_t> m_peer_timestamps;
    std::vector<bool> m_peer_stable;
    std::vector<size_t> m_outdated_idx;
    std::vector<size_t> m_unstable_idx;

    ServerPolicy m_policy;
    std::array<uint8_t, REGISTER_SIZE> m_value;

    bool m_get_round_done = false;
    bool m_call_done = false;
};

enum class PutNextAction {
    None,
    FallbackPolicyRead,
    DoWrite,
    ReturnToClient,
};

class PutCallContext {
   public:
    PutCallContext(peer* client, int32_t client_id, int64_t ticket, int64_t key,
                   Value const* value)
        : m_client(client),
          m_client_id(client_id),
          m_ticket(ticket),
          m_key(key) {
        for (size_t idx = 0; idx < REGISTER_SIZE; ++idx) {
            m_value[idx] = value->data()->Get(idx);
        }
    }

    // add a new reply
    PutNextAction add_get_reply(int64_t policy_version, int64_t timestamp,
                                bool suspicious, ServerPolicy const& policy);
    PutNextAction add_put_reply();

    // whether the call is done
    inline bool get_done() const { return m_get_round_done; }
    inline bool call_done() const { return m_call_done; }

    // getters
    inline int64_t ticket() const { return m_ticket; }
    inline int64_t key() const { return m_key; }
    inline int64_t policy_version() const { return m_policy_version; }
    inline int64_t timestamp() const { return m_timestamp; }
    inline int64_t next_timestamp() const {
        uint64_t seqno = (m_timestamp < 0)
                             ? 1
                             : (static_cast<uint64_t>(m_timestamp) >> 32) + 1;
        return static_cast<int64_t>((seqno << 32) |
                                    static_cast<uint64_t>(m_client_id));
    }
    inline peer* client() const { return m_client; }
    inline int32_t client_id() const { return m_client_id; }

    inline ServerPolicy const& policy() const& { return m_policy; }
    inline std::array<uint8_t, REGISTER_SIZE> const& value() const& {
        return m_value;
    }

   private:
    peer* m_client = nullptr;
    int64_t m_ticket = -1;
    int64_t m_key = -1;
    int32_t m_client_id = -1;

    size_t m_n_suspicions = 0;
    size_t m_n_get_replies = 0;
    size_t m_n_put_replies = 0;
    int64_t m_policy_version = -1;
    int64_t m_timestamp = -1;

    ServerPolicy m_policy;
    std::array<uint8_t, REGISTER_SIZE> m_value;

    bool m_get_round_done = false;
    bool m_call_done = false;
};

class CallMap {
   public:
    GetCallContext* add_get_call(peer* client, int32_t client_id,
                                 int64_t ticket, int64_t key);
    PutCallContext* add_put_call(peer* client, int32_t client_id,
                                 int64_t ticket, int64_t key,
                                 Value const* value);

    GetCallContext* get_get_ctx(int64_t ticket);
    PutCallContext* get_put_ctx(int64_t ticket);

    void resolve_call(int64_t ticket);

   private:
    std::unordered_map<int64_t, GetCallContext> m_get_map;
    std::unordered_map<int64_t, PutCallContext> m_put_map;
};

}  // namespace teems
