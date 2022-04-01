#include "call_map.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "log.h"

namespace register_sgx {
namespace crash {

void CallMap::add_call(peer* client, int64_t ticket) {
    m_map.emplace(ticket, CallContext(client, ticket));
}

void CallMap::resolve_call(int64_t ticket) {
    auto it = m_map.find(ticket);
    if (it == std::end(m_map)) {
        ERROR("failed to find call for ticket %zd", ticket);
        return;
    }

    flatbuffers::FlatBufferBuilder builder;

    m_map.erase(it);
}

}  // namespace crash
}  // namespace register_sgx
