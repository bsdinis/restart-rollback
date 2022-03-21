/**
 * call_map.h
 *
 * The map of outstanding calls
 */

#pragma once
#include <unordered_map>
#include "peer.h"

namespace paxos_sgx {
namespace crash {

struct CallContext {
    CallContext(peer* clnt, int64_t ticket)
        : m_client(clnt), m_ticket(ticket) {}

    peer* m_client;
    int64_t m_ticket;
};

class CallMap {
   public:
    void add_call(size_t slot_n, peer* client, int64_t ticket);
    void resolve_call(size_t slot_n, int64_t account, int64_t amount,
                      bool success);

   private:
    std::unordered_map<size_t, CallContext> m_map;
};

}  // namespace crash
}  // namespace paxos_sgx
