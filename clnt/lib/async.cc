#include "async.h"

#include "context.h"
#include "log.h"
#include "metadata.h"
#include "network.h"
#include "peer.h"
#include "result.h"
#include "untrusted.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace teems {

//===========================
// GLOBALS
//===========================

::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;

int64_t g_call_ticket = 0;
size_t g_calls_issued = 0;
size_t g_calls_concluded = 0;

//===========================
// EXTERN
//===========================
extern timeval g_timeout;
extern ::std::vector<peer> g_servers;

//===========================
// HELPERS DECLARATION
//===========================
namespace {
int64_t gen_subcall_ticket(int64_t teems_ticket, uint8_t call_number,
                           bool independent, call_type type,
                           call_target target);
}  // anonymous namespace

//===========================
// LIBRARY IMPLEMENTATION
//===========================

int async_init(size_t concurrent_hint) {
    g_results_map.reserve(concurrent_hint);
    return 0;
}
int async_close() {
    g_results_map.clear();
    return 0;
}

/// Ticket Semantics
///
/// A ticket identifies a call.
/// However, it is also used to encode some information about the call.
///
/// As such, the bit pattern has meaning.
/// There are two main classes of tickets:
///   - call tickets, for top-level TEEMS calls
///   - subcall tickets, for calls made from TEEMS to
///     either the metadata or untrusted storage subsystems
///
/// A call ticket has the following bit pattern
///  64            18   16                0
///   |ticket number|type|0000000000000000|
///
/// Where the type 2-bit pattern is one of the following:
///     01: sync call
///     10: async call
///     11: callback call
///
///     the 00 pattern is invalid
///
/// A subcall ticket has the following bit pattern
///  64            18   16           8    5           4      3        1 0
///   |ticket number|type|call number|xxxx|independent|target|sub type|1|
///
/// The higher level bits are exactly the same as the top-level call.
/// This has two consequences:
///     1. We need the top level call ticket to generate the subcall ticket
///     2. We can recover the top level call ticket from the subcall ticket
///
/// The first  bit indicates that this is a sub call ticket
/// The second two bit indicates the sub call type:
///     0b01: sync sub call
///     0b10: async sub call
///     0b11: callback sub call
///
/// The fifth bit indicates the subsystem the sub call is targetting:
///     0: metadata subsystem
///     1: untrusted storage subsystem
///
/// The fourth bit indicates whether the subsystem call is independent (was made
/// directly) or whether it was issued by TEEMS. This is important to recognize
/// if we need to run the continuation of the protocol or whether we place the
/// result in the results map.
///
/// The second byte includes a call number, because a top-level call may
/// need to make more than one call to the same subsystem.

int64_t gen_teems_ticket(call_type type) {
    uint64_t ticket = static_cast<uint64_t>(g_call_ticket);
    uint64_t t_number = ticket >> 18;
    t_number += 1;

    uint8_t t_type = 0;
    switch (type) {
        case call_type::Sync:
            t_type = 0b01;
            break;
        case call_type::Async:
            t_type = 0b10;
            break;
        case call_type::Callback:
            t_type = 0b11;
            break;
    }

    g_call_ticket = static_cast<int64_t>((t_number << 18) | (t_type << 16));
    return g_call_ticket;
}

int64_t gen_metadata_ticket(int64_t teems_ticket, uint8_t call_number,
                            bool independent, call_type type) {
    return gen_subcall_ticket(teems_ticket, call_number, independent, type,
                              call_target::Metadata);
}
int64_t gen_untrusted_ticket(int64_t teems_ticket, uint8_t call_number,
                             bool independent, call_type type) {
    return gen_subcall_ticket(teems_ticket, call_number, independent, type,
                              call_target::Untrusted);
}
int64_t get_supercall_ticket(int64_t subcall_ticket) {
    uint64_t u_ticket = static_cast<uint64_t>(subcall_ticket) & (~0x3ff);
    return static_cast<int64_t>(u_ticket);
}

bool ticket_independent(int64_t ticket) {
    uint64_t u_ticket = static_cast<uint64_t>(ticket);
    if (ticket_call_target(ticket) == call_target::TEEMS) {
        ERROR(
            "all TEEMS calls are independent: calling this is probably a logic "
            "error [ticket %lx]",
            ticket);
        return true;
    }

    return ((u_ticket >> 4) & 0b1) == 0b1;
}

call_type ticket_call_type(int64_t ticket) {
    uint64_t u_ticket = static_cast<uint64_t>(ticket);
    if (ticket_call_target(ticket) == call_target::TEEMS) {
        uint8_t u_type = static_cast<uint8_t>((u_ticket >> 16) & 0b11);
        switch (u_type) {
            case 0b01:
                return call_type::Sync;
            case 0b10:
                return call_type::Async;
            case 0b11:
                return call_type::Callback;
            default:
                ERROR("invalid TEEMS call type: %x", u_type);
                return call_type::Sync;
        }
    } else {
        switch ((u_ticket >> 1) & 0b11) {
            case 0b01:
                return call_type::Sync;
            case 0b10:
                return call_type::Async;
            case 0b11:
                return call_type::Callback;
            default:
                ERROR("invalid sub call type: %x", 0b00);
                return call_type::Sync;
        }
    }
}

call_target ticket_call_target(int64_t ticket) {
    uint64_t u_ticket = static_cast<uint64_t>(ticket);
    if ((u_ticket & 0b1) == 0) {
        return call_target::TEEMS;
    }

    if (((u_ticket >> 3) & 0b1) == 0) {
        return call_target::Metadata;
    } else {
        return call_target::Untrusted;
    }
}

void issued_call(int64_t ticket) {
    g_calls_issued += 1;
    LOG("issued   %lx | %zd | %zd | %zd", ticket, g_calls_issued,
        g_calls_concluded, n_calls_outlasting());
}
void finished_call(int64_t ticket) {
    g_calls_concluded += 1;
    LOG("finished_call   %lx | %zd | %zd | %zd", ticket, g_calls_issued,
        g_calls_concluded, n_calls_outlasting());
}

size_t n_calls_issued() { return g_calls_issued; }
size_t n_calls_concluded() { return g_calls_concluded; }
size_t n_calls_outlasting() { return n_calls_issued() - n_calls_concluded(); }

bool has_result(int64_t ticket) {
    return g_results_map.find(ticket) != std::end(g_results_map);
}

// functions to advance state
poll_state poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }
    if (n_calls_outlasting() == 0) {
        return poll_state::NoCalls;
    }
    if (ticket == -1) {
        if (poll_metadata(-1) == poll_state::Error ||
            poll_untrusted(-1) == poll_state::Error) {
            return poll_state::Error;
        }
        return n_calls_outlasting() == 0 ? poll_state::NoCalls
                                         : poll_state::Pending;
    }

    int64_t metadata_ticket = -1;
    int64_t untrusted_ticket = -1;
    switch (ticket_call_target(ticket)) {
        case call_target::TEEMS:
            if (get_call_tickets(ticket, &metadata_ticket, &untrusted_ticket) !=
                0) {
                ERROR("failed to get call tickets for TEEMS ticket %lx",
                      ticket);
                return poll_state::Error;
            }
            if (metadata_ticket != -1) {
                if (poll_metadata(metadata_ticket) == poll_state::Error) {
                    return poll_state::Error;
                }
            }
            if (untrusted_ticket != -1) {
                if (poll_untrusted(untrusted_ticket) == poll_state::Error) {
                    return poll_state::Error;
                }
            }
            return has_result(ticket) ? poll_state::Ready : poll_state::Pending;
        case call_target::Metadata:
            return poll_metadata(ticket);
        case call_target::Untrusted:
            return poll_untrusted(ticket);
    }
}

poll_state wait_for(int64_t ticket) {
    if (ticket == -1) {
        poll_state res = poll_state::Pending;
        while (res != poll_state::NoCalls && res != poll_state::Error) {
            res = poll(-1);
        }

        return res;
    } else {
        while (poll(ticket) == poll_state::Pending)
            ;
        poll_state state = poll(ticket);
        if (state == poll_state::Error) {
            return poll_state::Error;
        } else if (state == poll_state::NoCalls) {
            ERROR("waiting for call that does not exist %lx", ticket);
            return poll_state::Error;
        }
        return poll_state::Ready;
    }
}

template <typename T>
T get_reply(int64_t ticket) {
    auto it = g_results_map.find(ticket);
    if (it == std::end(g_results_map)) {
        ERROR("failed to find reply for %lx", ticket);
        return T();
    }
    std::unique_ptr<result> res = std::move(it->second);  // mv constructor
    g_results_map.erase(it);

    if (res->type() != result_type::OneVal) {
        ERROR(
            "the default template implementation of %s() only works for OneVal "
            "results",
            __func__);
        ERROR("returning default constructed value");
        return T();
    }

    one_val_result<T> *downcasted =
        dynamic_cast<one_val_result<T> *>(res.get());
    return downcasted->get();
}

// for put
template std::tuple<int64_t, bool, int64_t, int64_t> get_reply(int64_t ticket);

// for get
template std::tuple<int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>
get_reply(int64_t ticket);

// for change policy
template std::tuple<int64_t, bool, int64_t> get_reply(int64_t ticket);

// for metadata put
template std::pair<int64_t, bool> get_reply(int64_t ticket);

// for metadata get
template std::tuple<int64_t, Metadata, int64_t> get_reply(int64_t ticket);

// for untrusted put
template bool get_reply(int64_t ticket);

// for untrusted get
template std::tuple<bool, std::vector<uint8_t>> get_reply(int64_t ticket);

// for ping reset
template <>
void get_reply(int64_t ticket) {
    auto it = g_results_map.find(ticket);
    if (it == std::end(g_results_map)) {
        ERROR("failed to find reply for %ld", ticket);
        return;
    }

    std::unique_ptr<result> res = std::move(it->second);
    if (res->type() != result_type::None)
        ERROR(
            "the template specialization of %s for None was called with "
            "another value",
            __func__);
    g_results_map.erase(it);
}

// eg: if you are adding an RPC like `double avg(std::vector<int64_t> & const
// v)` you should add the following line `template double get_reply(int64_t
// double)`

//===========================
// HELPERS IMPLEMENTATION
//===========================
namespace {

int64_t gen_subcall_ticket(int64_t teems_ticket, uint8_t call_number,
                           bool independent, call_type type,
                           call_target target) {
    if (target == call_target::TEEMS) {
        ERROR("cannot create a subcall for TEEMS");
        return -1;
    }
    if ((static_cast<uint64_t>(teems_ticket) & 0xffff) != 0) {
        ERROR("invalid TEEMS ticket: %lx", static_cast<uint64_t>(teems_ticket));
        return -1;
    }

    uint16_t m_ticket = static_cast<uint16_t>(call_number);
    uint16_t m_independent = (independent) ? 0b1 : 0b0;
    uint16_t m_target = (target == call_target::Metadata) ? 0b0 : 0b1;
    uint16_t m_type = 0b00;
    switch (type) {
        case call_type::Sync:
            m_type = 0b01;
            break;
        case call_type::Async:
            m_type = 0b10;
            break;
        case call_type::Callback:
            m_type = 0b11;
            break;
    }

    uint64_t ticket = static_cast<uint64_t>(teems_ticket) | m_ticket << 8 |
                      m_independent << 4 | m_target << 3 | m_type << 1 | 0b1;
    return static_cast<int64_t>(ticket);
}

}  // anonymous namespace
}  // namespace teems
