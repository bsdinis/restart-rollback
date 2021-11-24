/**
 * qp_clnt.h
 *
 * client library to interface with a basicQP
 *
 * there are three different APIs: a sync API, an async API and a callback API.
 *
 * the sync API acts like an RPC: it is a transparent function call
 *
 * the async API returns a ticket.
 *   poll(ticket) can be used to advance the execution.
 *   wait_for(ticket) blocks on the call
 *   get_reply(ticket) retrieves (and erases) the result of a certain computation
 *
 * the callback API allows for callbacks to be registered to handle the results
 *   when a reply arrives whatever callback is registered *at that time* is employed.
 *   by default does nothing
 */

#pragma once

#include <sys/time.h>
#include <cstddef>
#include <cstdint>

#include <vector>
#include <functional>

namespace epidemics{
namespace basicQP {

// replies from the `poll` method
enum class poll_state {
    READY = 0,       // the reply is available
    PENDING = 1,     // the call is not ready, call selectable
    ERR = 2,         // the connection broke, give up
    NO_CALLS = 2,    // no outlasting calls
};


// initialize the connection
int init(
        char const * config = "../QP/default.conf",
        ssize_t idx = 0,
        size_t concurrent_hint = 1<<15,   // hint for number of concurrent calls permitted
        struct timeval timeout = { 0, 0 }, // timeout for select
        char const * cert_path = "certs/client.crt",
        char const * key_path = "certs/client.key"
        );

// close the connection
int close(bool close_remote = false);

/**
 * information about the calls made
 */
size_t n_calls_issued();
size_t n_calls_concluded();
size_t n_calls_outlasting();

/**
 * sync API
 *
 * RPC-style calls
 * XXX: CHANGE ME
 * add new sync calls
 */
int64_t sum(std::vector<int64_t> const &);
void ping(void);

/**
 * async API
 *
 * async RPC: calls return a ticket identifying the call
 * that ticket can be passed (repeatedly) to poll until it returns READY
 * alternatively, the caller can block until a ticket arrives with wait_for
 *
 * when a reply is ready, get_reply can be called (at most once per ticket) to retrieve the result
 *
 * XXX: CHANGE ME
 * add new async calls
 */
int64_t sum_async(std::vector<int64_t> const &);
int64_t ping_async(void);

poll_state poll(int64_t ticket = -1);
poll_state wait_for(int64_t ticket = -1);
template<typename T>
T get_reply(int64_t ticket);

/**
 * callback API
 *
 * can register callbacks for each function
 *
 * a callback for a function like `int f()` should have the following type signature:
 *    `void f_callback(int64_t ticket, int res)`
 *
 * functions need to be called in a special manner
 * calling poll with no arguments advances all calls; when there are no more outlasting calls poll returns NO_CALLS
 * there is also an equivalent for wait_for
 *
 * XXX: CHANGE ME
 * add new callbacks and callback setters
 */

// -1 means error
int sum_set_cb(std::function<void(int64_t, int64_t)> cb);
int64_t sum_cb(std::vector<int64_t> const &);

int ping_set_cb(std::function<void(int64_t)> cb);
int64_t ping_cb(void);

} // namespace basicQP
} // namespace epidemics
