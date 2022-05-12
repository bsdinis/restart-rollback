#pragma once

#include <cstddef>
#include <cstdint>

namespace teems{

// replies from the `poll` method
enum class poll_state {
    READY = 0,       // the reply is available
    PENDING = 1,     // the call is not ready, call selectable
    ERR = 2,         // the connection broke, give up
    NO_CALLS = 2,    // no outlasting calls
};

// type of call
enum class call_type { Sync, Async, Callback };
enum class call_target { TEEMS, Untrusted, Metadata };

int async_init(size_t concurrent_hint = 1<<15);
int async_close();

// ticket numbers have semantic
int64_t gen_teems_ticket(call_type type);
int64_t gen_metadata_ticket(int64_t teems_ticket, uint8_t call_number, call_type type);
int64_t gen_untrusted_ticket(int64_t teems_ticket, uint8_t call_number, call_type type);

call_type ticket_call_type(int64_t ticket);
call_target ticket_call_target(int64_t ticket);

// information about the calls made
size_t n_calls_issued();
size_t n_calls_concluded();
size_t n_calls_outlasting();

bool has_result(int64_t ticket);
poll_state poll(int64_t ticket = -1);
poll_state wait_for(int64_t ticket = -1);
template<typename T>
T get_reply(int64_t ticket);

} // namespace teems
