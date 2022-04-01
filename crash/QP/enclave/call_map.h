/**
 * call_map.h
 *
 * The map of outstanding calls
 */

#pragma once
#include <unordered_map>
#include "peer.h"

namespace register_sgx {
namespace crash {

struct CallContext {
    CallContext(peer* clnt, int64_t ticket)
        : m_client(clnt), m_ticket(ticket) {}

    peer* m_client;
    int64_t m_ticket;
};

class CallMap {
   public:
    void add_call(peer* client, int64_t ticket);
    void resolve_call(int64_t ticket);

   private:
    std::unordered_map<int64_t, CallContext> m_map;
};

}  // namespace crash
}  // namespace register_sgx
