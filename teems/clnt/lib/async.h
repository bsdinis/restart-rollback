#pragma once

#include <cstddef>
#include <cstdint>

namespace teems {

// replies from the `poll` method
enum class poll_state { Ready, Pending, NoCalls, Error };

// type of call
enum class call_type { Sync, Async, Callback };
enum class call_target { TEEMS, Untrusted, Metadata };

int async_init(size_t concurrent_hint = 1 << 15);
int async_close();

// ticket numbers have semantic
int64_t gen_teems_ticket(call_type type);
int64_t gen_metadata_ticket(int64_t teems_ticket, uint8_t call_number,
                            bool independent, call_type type);
int64_t gen_untrusted_ticket(int64_t teems_ticket, uint8_t call_number,
                             bool independent, call_type type);
int64_t get_supercall_ticket(int64_t subcall_ticket);

bool ticket_independent(int64_t ticket);
call_type ticket_call_type(int64_t ticket);
call_target ticket_call_target(int64_t ticket);

// information about the calls made
void issued_call(int64_t ticket);
void finished_call(int64_t ticket);
size_t n_calls_issued();
size_t n_calls_concluded();
size_t n_calls_outlasting();

bool has_result(int64_t ticket);
poll_state poll(int64_t ticket = -1);
poll_state wait_for(int64_t ticket = -1);

// TODO: all instantiations
void put_reply(int64_t ticket, ...);
template <typename T>
T get_reply(int64_t ticket);

}  // namespace teems
