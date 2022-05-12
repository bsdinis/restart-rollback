#pragma once

#include <cstdint>

namespace teems {

// replies from the `poll` method
enum class poll_state {
    READY = 0,       // the reply is available
    PENDING = 1,     // the call is not ready, call selectable
    ERR = 2,         // the connection broke, give up
    NO_CALLS = 2,    // no outlasting calls
};

// type of call
enum class call_type { SYNC, ASYNC, CALLBACK };

extern int64_t g_call_ticket;

// ticket numbers have semantic.
// specifically, the remainder mod 3 indicates
// which type of call they relate to
inline int64_t gen_ticket(call_type type) {
    int const target = type == call_type::SYNC ? 0 : (type == call_type::ASYNC ? 1 : 2);
    g_call_ticket += 3 - (g_call_ticket % 3) + target;
    return g_call_ticket;
}


} // namespace teems
