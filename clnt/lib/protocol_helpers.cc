#include "protocol_helpers.h"

#include "async.h"
#include "cache.h"
#include "context.h"
#include "log.h"
#include "metadata.h"
#include "network.h"
#include "result.h"
#include "teems_config.h"
#include "teems_generated.h"
#include "untrusted.h"

#include <algorithm>
#include <unordered_map>

namespace teems {

extern int32_t g_client_id;

extern ::std::vector<peer> g_servers;

extern std::function<void(int64_t, int64_t, bool, std::vector<uint8_t>, int64_t,
                          int64_t)>
    g_get_callback;
extern std::function<void(int64_t, int64_t, bool, int64_t, int64_t)>
    g_put_callback;
extern std::function<void(int64_t, int64_t, bool, int64_t)>
    g_change_policy_callback;
extern std::function<void(int64_t)> g_ping_callback;
extern std::function<void(int64_t)> g_reset_callback;

extern ::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;

// global variables to channel the results of synchronous calls to

namespace {
int greeting_handler(size_t peer_idx, int32_t id);

int ping_handler(size_t peer_idx, int64_t ticket);
int ping_handler_sync();
int ping_handler_async(int64_t ticket);

int reset_handler(size_t peer_idx, int64_t ticket);
int reset_handler_sync();
int reset_handler_async(int64_t ticket);

int teems_finish_get(int64_t ticket, int64_t key, bool success,
                     std::vector<uint8_t> const &value = std::vector<uint8_t>(),
                     int64_t policy_version = -1, int64_t timestamp = -1,
                     std::string const &ustor_name = std::string());
int teems_finish_put(int64_t ticket, int64_t key, bool success,
                     int64_t policy_version = -1, int64_t timestamp = -1,
                     std::string ustor_name = std::string(),
                     std::vector<uint8_t> value = std::vector<uint8_t>());

}  // anonymous namespace

// =================================
// send request
// =================================
int64_t send_ping_request(peer &server, call_type type) {
    int64_t const ticket =
        gen_metadata_ticket(gen_teems_ticket(type), 0, true, type);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_args = teems::CreateEmpty(builder);

    auto request =
        teems::CreateMessage(builder, teems::MessageType_ping_req, ticket,
                             teems::BasicMessage_Empty, ping_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    issued_call(ticket);
    return ticket;
}

int64_t send_reset_request(peer &server, call_type type) {
    int64_t const ticket =
        gen_metadata_ticket(gen_teems_ticket(type), 0, true, type);

    flatbuffers::FlatBufferBuilder builder;
    auto reset_args = teems::CreateEmpty(builder);

    auto request =
        teems::CreateMessage(builder, teems::MessageType_reset_req, ticket,
                             teems::BasicMessage_Empty, reset_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    issued_call(ticket);
    return ticket;
}

// =================================
// continuations
// =================================

int teems_get_final_step(int64_t super_ticket, int64_t key, get_call_ctx *ctx) {
    std::vector<uint8_t> value;
    if (ctx->metadata().decrypt_value(ctx->encrypted_value(), value) == false) {
        ERROR("get(%ld): failed to decrypt value", key);
        if (teems_finish_get(super_ticket, key, false) != 0) {
            ERROR("get(%ld): failed to finish TEEM get", key);
            return -1;
        }
        return 0;
    }

    return teems_finish_get(super_ticket, key, true, std::move(value),
                            ctx->policy_version(), ctx->timestamp(),
                            ctx->metadata().ustor_name());
}

int teems_handle_metadata_get(int64_t ticket, int64_t key, Metadata value,
                              int64_t policy_version, int64_t timestamp,
                              bool success) {
    int64_t super_ticket = get_supercall_ticket(ticket);

    get_call_ctx *ctx = get_get_call_ctx(super_ticket);
    if (ctx == nullptr) {
        ERROR("failed to find context for TEEM get call %ld", super_ticket);
        return -1;
    }

    if (!success) {
        ERROR("get(%ld): failed to read metadata from TEEMS", key);
        if (teems_finish_get(super_ticket, key, false) != 0) {
            ERROR("get(%ld): failed to finish TEEM get", key);
            return -1;
        }
        return 0;
    }

    if (timestamp == -1) {
        if (teems_finish_get(super_ticket, key, true, std::vector<uint8_t>(),
                             -1, -1, "") != 0) {
            ERROR("get(%ld): failed to finish TEEM get", key);
            return -1;
        }
        return 0;
    }

    ctx->set_metadata(std::move(value), policy_version, timestamp);

    if (ctx->metadata().ustor_name() == ctx->name_hint()) {
        if (ctx->value_set()) {
            // value hint worked
            return teems_finish_get(super_ticket, key, true, ctx->value(),
                                    ctx->policy_version(), ctx->timestamp(),
                                    ctx->metadata().ustor_name());
        } else if (ctx->encrypted_value_set()) {
            // name hint worked and arrived before metadata
            return teems_get_final_step(super_ticket, key, ctx);
        } else {
            // name hint worked and is on the way
            return 0;
        }
    }

    int64_t const untrusted_ticket =
        gen_untrusted_ticket(super_ticket, 1, false, call_type::Async);
    ctx->set_untrusted_ticket(untrusted_ticket);

    std::string const &ustor_name = ctx->metadata().ustor_name();
    if (untrusted_get_async(super_ticket, 1, false, ustor_name) == -1) {
        ERROR("get(%ld): failed to start untrusted read", key);
        if (teems_finish_get(super_ticket, key, false) != 0) {
            ERROR("get(%ld): failed to finish TEEM get", key);
            return -1;
        }
        return -1;
    }

    return 0;
}
int teems_handle_metadata_put(int64_t ticket, int64_t policy_version,
                              int64_t timestamp, bool success) {
    int64_t super_ticket = get_supercall_ticket(ticket);

    put_call_ctx *ctx = get_put_call_ctx(super_ticket);
    if (ctx == nullptr) {
        ERROR("failed to find context for TEEM put call %ld", super_ticket);
        return -1;
    }
    int64_t key = ctx->key();
    auto name = ctx->metadata().ustor_name();
    auto value = ctx->value();

    if (!success) {
        ERROR("put(%ld): failed to write metadata to TEEMS", key);
        if (teems_finish_put(super_ticket, key, false) != 0) {
            ERROR("put(%ld): failed to finish TEEM put", key);
            return -1;
        }
        return 0;
    }

    return teems_finish_put(super_ticket, key, true, policy_version, timestamp,
                            ctx->metadata().ustor_name(), ctx->value());
}

int teems_handle_untrusted_get(int64_t ticket, bool success,
                               std::vector<uint8_t> &&encrypted_value) {
    int64_t super_ticket = get_supercall_ticket(ticket);
    get_call_ctx *ctx = get_get_call_ctx(super_ticket);
    if (ctx == nullptr) {
        ERROR("failed to find context for TEEM get call %ld", super_ticket);
        return -1;
    }
    int64_t key = ctx->key();

    if (!success) {
        ERROR("get(%ld): failed to read encrypted value from untrusted storage",
              key);
        if (teems_finish_get(super_ticket, key, false) != 0) {
            ERROR("get(%ld): failed to finish TEEM get", key);
            return -1;
        }
        return 0;
    }

    ctx->set_encrypted_value(std::move(encrypted_value));

    if (ctx->metadata_set()) {
        return teems_get_final_step(super_ticket, key, ctx);
    }

    return 0;
}
int teems_handle_untrusted_put(int64_t ticket, bool success) {
    int64_t super_ticket = get_supercall_ticket(ticket);

    put_call_ctx *ctx = get_put_call_ctx(super_ticket);
    if (ctx == nullptr) {
        ERROR("failed to find context for TEEM put call %ld", super_ticket);
        return -1;
    }
    int64_t key = ctx->key();

    if (!success) {
        ERROR("put(%ld): failed to write encrypted value to untrusted storage",
              key);
        if (teems_finish_put(super_ticket, key, false, -1, -1) != 0) {
            ERROR("put(%ld): failed to finish TEEM put", key);
            return -1;
        }
        return 0;
    }

    int64_t const metadata_ticket =
        gen_metadata_ticket(super_ticket, 0, false, call_type::Async);
    ctx->set_metadata_ticket(metadata_ticket);
    if (metadata_put_async(super_ticket, 0, false, key, ctx->metadata()) ==
        -1) {
        ERROR("put(%ld): failed to start metadata write to TEEMS", key);
        if (teems_finish_put(super_ticket, key, false, -1, -1) != 0) {
            ERROR("put(%ld): failed to finish TEEM put", key);
            return -1;
        }
        return -1;
    }

    return 0;
}

// =================================
// high level message handler
// =================================

int handle_received_message(size_t idx, peer &p) {
    FINE("received a message from server %zu: %zu B", idx, p.buffer().size());
    while (p.buffer().size() > 0) {
        size_t const total_size = *(size_t *)(p.buffer().data());
        FINE("resp size: %zu B (according to header)", total_size);
        if (total_size + sizeof(size_t) > p.buffer().size()) return 0;

        auto response = teems::GetMessage(p.buffer().data() + sizeof(size_t));

        switch (response->type()) {
            case teems::MessageType_client_greeting:
                FINE("greeting: %d", response->message_as_Greeting()->id());
                greeting_handler(idx, response->message_as_Greeting()->id());
                break;
            case teems::MessageType_proxy_get_resp:
                FINE("proxy get response [ticket %ld]", response->ticket());
                metadata_get_handler(
                    idx, response->ticket(),
                    response->message_as_GetResult()->key(),
                    Metadata(response->message_as_GetResult()->value()->data()),
                    response->message_as_GetResult()->policy_version(),
                    response->message_as_GetResult()->timestamp());
                break;
            case teems::MessageType_proxy_put_resp:
                FINE("proxy put response [ticket %ld]", response->ticket());
                metadata_put_handler(
                    idx, response->ticket(),
                    response->message_as_PutResult()->success(),
                    response->message_as_PutResult()->policy_version(),
                    response->message_as_PutResult()->timestamp());
                break;
            case teems::MessageType_ping_resp:
                FINE("ping response [ticket %ld]", response->ticket());
                ping_handler(idx, response->ticket());
                break;
            case teems::MessageType_reset_resp:
                FINE("reset response [ticket %ld]", response->ticket());
                reset_handler(idx, response->ticket());
                break;
            default:
                ERROR("Unknown response %d [ticket %ld]", response->type(),
                      response->ticket());
                break;
        }
        p.skip(sizeof(size_t) + total_size);
    }
    return 0;
}

namespace {

int greeting_handler(size_t peer_idx, int32_t id) {
    if (id >= 0) {
        g_client_id = id;
    }

    return 0;
}

// ping
int ping_handler_sync() { return 0; }
int ping_handler_async(int64_t ticket) {
    g_results_map.emplace(ticket, std::make_unique<result>());
    return 0;
}
int ping_handler(size_t peer_idx, int64_t ticket) {
    finished_call(ticket);
    switch (ticket_call_type(ticket)) {
        case call_type::Sync:
            return ping_handler_sync();
        case call_type::Async:
            return ping_handler_async(ticket);
        case call_type::Callback:
            g_ping_callback(ticket);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// reset
int reset_handler_sync() { return 0; }
int reset_handler_async(int64_t ticket) {
    g_results_map.emplace(ticket, std::make_unique<result>());
    return 0;
}
int reset_handler(size_t peer_idx, int64_t ticket) {
    finished_call(ticket);
    switch (ticket_call_type(ticket)) {
        case call_type::Sync:
            return reset_handler_sync();
        case call_type::Async:
            return reset_handler_async(ticket);
        case call_type::Callback:
            g_reset_callback(ticket);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

int teems_finish_get(int64_t ticket, int64_t key, bool success,
                     std::vector<uint8_t> const &value, int64_t policy_version,
                     int64_t timestamp, std::string const &ustor_name) {
    finished_call(ticket);

    if (success) {
        add_hint(key, ustor_name, value);
    }

    if (ticket_call_type(ticket) == call_type::Async) {
        g_results_map.emplace(
            ticket,
            std::unique_ptr<result>(
                std::make_unique<one_val_result<std::tuple<
                    int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>>(
                    one_val_result<std::tuple<
                        int64_t, bool, std::vector<uint8_t>, int64_t, int64_t>>(
                        std::make_tuple(key, success, std::move(value),
                                        policy_version, timestamp)))));
    } else {
        g_get_callback(ticket, key, success, std::move(value), policy_version,
                       timestamp);
        return 0;
    }
    rem_get_call(ticket);
    return 0;
}
int teems_finish_put(int64_t ticket, int64_t key, bool success,
                     int64_t policy_version, int64_t timestamp,
                     std::string ustor_name, std::vector<uint8_t> value) {
    finished_call(ticket);
    if (success) {
        add_hint(key, ustor_name, value);
    }

    if (ticket_call_type(ticket) == call_type::Async) {
        g_results_map.emplace(
            ticket,
            std::unique_ptr<result>(
                std::make_unique<one_val_result<
                    std::tuple<int64_t, bool, int64_t, int64_t>>>(
                    one_val_result<std::tuple<int64_t, bool, int64_t, int64_t>>(
                        std::make_tuple(key, success, policy_version,
                                        timestamp)))));
    } else {
        g_put_callback(ticket, key, success, policy_version, timestamp);
    }
    return 0;
    rem_put_call(ticket);
}

}  // anonymous namespace
}  // namespace teems
