/**
 * teems.h
 *
 * TEEMS library
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
#include <sys/types.h>
#include <cstddef>
#include <cstdint>

#include <vector>
#include <functional>

#include "async.h"
#include "untrusted.h"

namespace teems{

// initialize the connection
int init(
        char const * config = "../server/default.conf",
        size_t concurrent_hint = 1<<15,   // hint for number of concurrent calls permitted
        struct timeval timeout = { 0, 0 }, // timeout for select
        char const * cert_path = "certs/client.crt",
        char const * key_path = "certs/client.key",
        UntrustedStoreType ustor_type = UntrustedStoreType::S3
        );

// close the connection
int close(bool close_remote = false);

// number of servers we are connected to
size_t n_servers();


int32_t client_id();

/**
 * sync API
 *
 * RPC-style calls
 */
bool get(int64_t key, std::vector<uint8_t> &value, int64_t &policy_version, int64_t &timestamp);
bool put(int64_t key, std::vector<uint8_t> const&value, int64_t &policy_version, int64_t &timestamp);
bool change_policy(int64_t key, uint64_t policy, int64_t &policy_version);

void ping(void);
void reset(void);

/**
 * async API
 *
 * async RPC: calls return a ticket identifying the call
 * that ticket can be passed (repeatedly) to poll until it returns READY
 * alternatively, the caller can block until a ticket arrives with wait_for
 *
 * when a reply is ready, get_reply can be called (at most once per ticket) to retrieve the result
 *
 */

int64_t get_async(int64_t key);
int64_t put_async(int64_t key, std::vector<uint8_t> const&value);
int64_t change_policy_async(int64_t key, uint64_t policy);

int64_t ping_async(void);
int64_t reset_async(void);

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
 */

// -1 means error
int get_set_cb(std::function<void(int64_t, int64_t, bool, std::vector<uint8_t>, int64_t, int64_t)> cb);
int64_t get_cb(int64_t key);

int put_set_cb(std::function<void(int64_t, int64_t, bool, int64_t, int64_t)> cb);
int64_t put_cb(int64_t key, std::vector<uint8_t> const &value);

int change_policy_set_cb( std::function<void(int64_t, int64_t, bool, int64_t)> cb);
int64_t change_policy_cb(int64_t key, uint64_t policy);

int ping_set_cb(std::function<void(int64_t)> cb);
int64_t ping_cb(void);

int reset_set_cb(std::function<void(int64_t)> cb);
int64_t reset_cb(void);

} // namespace teems
