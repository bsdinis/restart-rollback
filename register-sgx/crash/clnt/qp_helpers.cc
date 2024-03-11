#include "qp_helpers.h"
#include "qp_context.h"
#include "qp_network.h"
#include "qp_result.h"

#include "crash_generated.h"
#include "log.h"

#include <algorithm>
#include <unordered_map>

namespace register_sgx {
namespace crash {

extern int32_t g_client_id;

extern ::std::vector<peer> g_servers;
extern size_t g_calls_issued;
extern size_t g_calls_concluded;

extern GetContext g_get_sync_ctx;
extern PutContext g_put_sync_ctx;

extern ::std::unordered_map<int64_t, GetContext> g_get_ctx_map;
extern ::std::unordered_map<int64_t, PutContext> g_put_ctx_map;

extern std::function<void(int64_t, int64_t, std::array<uint8_t, REGISTER_SIZE>,
                          int64_t, bool)>
    g_get_callback;
extern std::function<void(int64_t, bool, int64_t)> g_put_callback;
extern std::function<void(int64_t, int64_t, std::array<uint8_t, REGISTER_SIZE>,
                          int64_t)>
    g_proxy_get_callback;
extern std::function<void(int64_t, bool, int64_t)> g_proxy_put_callback;
extern std::function<void(int64_t)> g_ping_callback;
extern std::function<void(int64_t)> g_reset_callback;

// global variables to channel the results of synchronous calls to

// get
int64_t g_get_key_result = -1;
int64_t g_get_timestamp_result = -1;
::std::array<uint8_t, REGISTER_SIZE> g_get_value_result;
bool g_get_success_result = false;
int64_t g_get_sync_ticket = -1;

// put
int64_t g_put_timestamp_result;
bool g_put_success_result = false;
int64_t g_put_sync_ticket = -1;

// proxy get
std::tuple<int64_t, std::array<uint8_t, REGISTER_SIZE>, int64_t>
    g_proxy_get_result;

// proxy put
std::pair<int64_t, bool> g_proxy_put_result;

// async results
::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;

namespace {
inline size_t quorum_size() { return ((g_servers.size() - 1) / 2) + 1; }

int send_payload_to(peer &server, uint8_t const *payload, size_t size) {
    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    return 0;
}

int send_payload_to_all(uint8_t const *payload, size_t size) {
    for (ssize_t idx = 0; idx < g_servers.size(); ++idx) {
        auto &server = g_servers[idx];
        if (send_payload_to(server, payload, size) != 0) {
            ERROR("failed to send message to server %ld", idx);
            return -1;
        }
    }

    return 0;
}

int send_payload_to_list(std::vector<size_t> send_list, uint8_t const *payload,
                         size_t size) {
    std::sort(std::begin(send_list), std::end(send_list));
    auto it = std::unique(std::begin(send_list), std::end(send_list));
    send_list.resize(std::distance(std::begin(send_list), it));
    for (size_t idx : send_list) {
        if (idx >= g_servers.size()) {
            break;
        }

        if (send_payload_to(g_servers[idx], payload, size) != 0) {
            return -1;
        }
    }

    return 0;
}

inline bool is_get(int64_t ticket, call_type type) {
    if (type == call_type::SYNC) {
        return g_get_sync_ticket == ticket;
    }

    return g_get_ctx_map.find(ticket) != g_get_ctx_map.end();
}

inline bool is_put(int64_t ticket, call_type type) {
    if (type == call_type::SYNC) {
        return g_put_sync_ticket == ticket;
    }

    return g_put_ctx_map.find(ticket) != g_put_ctx_map.end();
}

inline bool operation_finished(int64_t ticket, call_type type) {
    if (type == call_type::SYNC) {
        return g_get_sync_ticket != ticket && g_put_sync_ticket != ticket;
    }

    return g_get_ctx_map.find(ticket) == g_get_ctx_map.end() &&
           g_put_ctx_map.find(ticket) == g_put_ctx_map.end();
}

call_type get_call_type(int64_t ticket);

int greeting_handler(size_t peer_idx, int32_t id);

int get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                std::array<uint8_t, REGISTER_SIZE> const &value,
                int64_t timestamp);
int get_timestamp_handler(size_t peer_idx, int64_t ticket, int64_t key,
                          int64_t timestamp);
GetContext *get_get_ctx(int64_t ticket, call_type type);
int get_protocol_get_round(GetContext &ctx, size_t peer_idx, int64_t ticket,
                           int64_t key,
                           std::array<uint8_t, REGISTER_SIZE> const &value,
                           int64_t timestamp, call_type type);
int get_protocol_writeback_round(GetContext &ctx, int64_t ticket,
                                 int64_t timestamp, bool success,
                                 call_type type);

int get_finish(int64_t ticket, int64_t key, int64_t timestamp,
               ::std::array<uint8_t, REGISTER_SIZE> value, bool success,
               call_type type);
int get_finish_sync(int64_t ticket, int64_t key, int64_t timestamp,
                    ::std::array<uint8_t, REGISTER_SIZE> value, bool success);
int get_finish_async(int64_t ticket, int64_t key, int64_t timestamp,
                     ::std::array<uint8_t, REGISTER_SIZE> value, bool success);
int get_finish_cb(int64_t ticket, int64_t key, int64_t timestamp,
                  ::std::array<uint8_t, REGISTER_SIZE> value, bool success);

int put_handler(size_t peer_idx, int64_t ticket, bool success,
                int64_t timestamp);
PutContext *get_put_ctx(int64_t ticket, call_type type);
int put_protocol_get_round(PutContext &ctx, int64_t ticket, int64_t key,
                           int64_t timestamp, call_type type);
int put_protocol_put_round(PutContext &ctx, int64_t ticket, int64_t timestamp,
                           bool success, call_type type);

int put_finish(int64_t ticket, int64_t timestamp, bool success, call_type type);
int put_finish_sync(int64_t ticket, int64_t timestamp, bool success);
int put_finish_async(int64_t ticket, int64_t timestamp, bool success);
int put_finish_cb(int64_t ticket, int64_t timestamp, bool success);

int proxy_get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                      std::array<uint8_t, REGISTER_SIZE> const &value,
                      int64_t timestamp);
int proxy_get_handler_sync(int64_t ticket, int64_t key,
                           std::array<uint8_t, REGISTER_SIZE> const &value,
                           int64_t timestamp);
int proxy_get_handler_async(int64_t ticket, int64_t key,
                            std::array<uint8_t, REGISTER_SIZE> const &value,
                            int64_t timestamp);

int proxy_put_handler(size_t peer_idx, int64_t ticket, bool success,
                      int64_t timestamp);
int proxy_put_handler_sync(int64_t ticket, bool success, int64_t timestamp);
int proxy_put_handler_async(int64_t ticket, bool success, int64_t timestamp);

int ping_handler(size_t peer_idx, int64_t ticket);
int ping_handler_sync();
int ping_handler_async(int64_t ticket);

int reset_handler(size_t peer_idx, int64_t ticket);
int reset_handler_sync();
int reset_handler_async(int64_t ticket);

}  // anonymous namespace

// =================================
// send request
// =================================
int64_t send_get_request(int64_t key, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto get_args = register_sgx::crash::CreateGetArgs(builder, key);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_req, ticket,
        register_sgx::crash::BasicMessage_GetArgs, get_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (send_payload_to_all(payload, size) != 0) {
        return -1;
    }

    g_calls_issued++;
    return ticket;
}

int64_t send_get_timestamp_request(int64_t key, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto get_args = register_sgx::crash::CreateGetTimestampArgs(builder, key);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_timestamp_req, ticket,
        register_sgx::crash::BasicMessage_GetTimestampArgs, get_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (send_payload_to_all(payload, size) != 0) {
        return -1;
    }

    g_calls_issued++;
    return ticket;
}

int send_put_request(int64_t ticket, int64_t key,
                     std::array<uint8_t, REGISTER_SIZE> const &value,
                     int64_t timestamp, call_type type) {
    flatbuffers::FlatBufferBuilder builder;

    auto fb_value = register_sgx::crash::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto put_args =
        register_sgx::crash::CreatePutArgs(builder, key, &fb_value, timestamp);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_put_req, ticket,
        register_sgx::crash::BasicMessage_PutArgs, put_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (send_payload_to_all(payload, size) != 0) {
        ERROR("failed to send put request to replicas [ticket %ld]", ticket);
        return -1;
    }

    return 0;
}

int send_writeback_request_to(int64_t ticket, int64_t key,
                              std::array<uint8_t, REGISTER_SIZE> const &value,
                              int64_t timestamp, call_type type,
                              std::vector<size_t> send_list) {
    flatbuffers::FlatBufferBuilder builder;

    auto fb_value = register_sgx::crash::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        ssize_t idx = 0;
        for (auto x : value) {
            fb_arr->Mutate(idx, x);
        }
    }

    auto put_args =
        register_sgx::crash::CreatePutArgs(builder, key, &fb_value, timestamp);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_put_req, ticket,
        register_sgx::crash::BasicMessage_PutArgs, put_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (send_payload_to_list(send_list, payload, size) != 0) {
        return -1;
    }

    return 0;
}

int64_t send_proxy_get_request(peer &server, int64_t key, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto get_args = register_sgx::crash::CreateGetArgs(builder, key);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_proxy_get_req, ticket,
        register_sgx::crash::BasicMessage_GetArgs, get_args.Union());
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

    g_calls_issued++;
    return ticket;
}

int64_t send_proxy_put_request(peer &server, int64_t key,
                               std::array<uint8_t, REGISTER_SIZE> const &value,
                               call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;

    auto fb_value = register_sgx::crash::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto put_args = register_sgx::crash::CreateProxyPutArgs(
        builder, key, &fb_value, g_client_id);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_proxy_put_req, ticket,
        register_sgx::crash::BasicMessage_ProxyPutArgs, put_args.Union());
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

    g_calls_issued++;
    return ticket;
}

int64_t send_ping_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_args = register_sgx::crash::CreateEmpty(builder);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_ping_req, ticket,
        register_sgx::crash::BasicMessage_Empty, ping_args.Union());
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

    g_calls_issued++;
    return ticket;
}

int64_t send_reset_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

    flatbuffers::FlatBufferBuilder builder;
    auto reset_args = register_sgx::crash::CreateEmpty(builder);

    auto request = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_reset_req, ticket,
        register_sgx::crash::BasicMessage_Empty, reset_args.Union());
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

    g_calls_issued++;
    return ticket;
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

        std::array<uint8_t, REGISTER_SIZE> value;
        auto response =
            register_sgx::crash::GetMessage(p.buffer().data() + sizeof(size_t));

        switch (response->type()) {
            case register_sgx::crash::MessageType_client_greeting:
                FINE("greeting: %d", response->message_as_Greeting()->id());
                greeting_handler(idx, response->message_as_Greeting()->id());
                break;
            case register_sgx::crash::MessageType_get_resp:
                FINE("get response [ticket %ld]", response->ticket());
                for (ssize_t idx = 0; idx < REGISTER_SIZE; ++idx) {
                    value[idx] =
                        response->message_as_GetResult()->value()->data()->Get(
                            idx);
                }
                get_handler(idx, response->ticket(),
                            response->message_as_GetResult()->key(), value,
                            response->message_as_GetResult()->timestamp());
                break;
            case register_sgx::crash::MessageType_get_timestamp_resp:
                FINE("get timestamp response [ticket %ld]", response->ticket());
                get_timestamp_handler(
                    idx, response->ticket(),
                    response->message_as_GetTimestampResult()->key(),
                    response->message_as_GetTimestampResult()->timestamp());
                break;
            case register_sgx::crash::MessageType_put_resp:
                FINE("put response [ticket %ld]", response->ticket());
                put_handler(idx, response->ticket(),
                            response->message_as_PutResult()->success(),
                            response->message_as_PutResult()->timestamp());
                break;
            case register_sgx::crash::MessageType_proxy_get_resp:
                FINE("proxy get response [ticket %ld]", response->ticket());
                for (ssize_t idx = 0; idx < REGISTER_SIZE; ++idx) {
                    value[idx] =
                        response->message_as_GetResult()->value()->data()->Get(
                            idx);
                }
                proxy_get_handler(
                    idx, response->ticket(),
                    response->message_as_GetResult()->key(), value,
                    response->message_as_GetResult()->timestamp());
                break;
            case register_sgx::crash::MessageType_proxy_put_resp:
                FINE("proxy put response [ticket %ld]", response->ticket());
                proxy_put_handler(
                    idx, response->ticket(),
                    response->message_as_PutResult()->success(),
                    response->message_as_PutResult()->timestamp());
                break;
            case register_sgx::crash::MessageType_ping_resp:
                FINE("ping response [ticket %ld]", response->ticket());
                ping_handler(idx, response->ticket());
                break;
            case register_sgx::crash::MessageType_reset_resp:
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

bool has_result(int64_t ticket) {
    return g_results_map.find(ticket) != std::end(g_results_map);
}

namespace {

call_type get_call_type(int64_t ticket) {
    switch (ticket % 3) {
        case 0:
            return call_type::SYNC;
        case 1:  // ASYNC
            return call_type::ASYNC;
        case 2:  // CALLBACK
            return call_type::CALLBACK;
    }
    __builtin_unreachable();
    return call_type::SYNC;
}

int greeting_handler(size_t peer_idx, int32_t id) {
    if (id >= 0) {
        g_client_id = id;
    }

    return 0;
}

int get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                std::array<uint8_t, REGISTER_SIZE> const &value,
                int64_t timestamp) {
    call_type const type = get_call_type(ticket);

    if (operation_finished(ticket, type)) {
        FINE("already finished operation with ticket %ld", ticket);
        return 0;
    }

    if (is_get(ticket, type)) {
        GetContext *ctx = get_get_ctx(ticket, type);
        if (ctx == nullptr) {
            ERROR("Failed to retrieve context for ticket %ld", ticket);
            return -1;
        }

        return get_protocol_get_round(*ctx, peer_idx, ticket, key, value,
                                      timestamp, type);
    }

    ERROR(
        "received a get response to an operation which 1: has not finished; 2: "
        "is not a get. [ticket %ld]",
        ticket);
    return -1;
}

int get_timestamp_handler(size_t peer_idx, int64_t ticket, int64_t key,
                          int64_t timestamp) {
    call_type const type = get_call_type(ticket);

    if (operation_finished(ticket, type)) {
        FINE("already finished operation with ticket %ld", ticket);
        return 0;
    }

    if (is_put(ticket, type)) {
        PutContext *ctx = get_put_ctx(ticket, type);
        if (ctx == nullptr) {
            ERROR("Failed to retrieve context for ticket %ld", ticket);
            return -1;
        }

        return put_protocol_get_round(*ctx, ticket, key, timestamp, type);
    }

    ERROR(
        "received a get timestamp response to an operation which 1: has not "
        "finished; 2: is not a put. [ticket %ld]",
        ticket);
    return -1;
}

GetContext *get_get_ctx(int64_t ticket, call_type type) {
    if (type == call_type::SYNC) {
        return (g_get_sync_ticket == ticket) ? &g_get_sync_ctx : nullptr;
    }

    auto it = g_get_ctx_map.find(ticket);
    if (it == g_get_ctx_map.end()) {
        return nullptr;
    }

    return &it->second;
}

int get_protocol_get_round(GetContext &ctx, size_t peer_idx, int64_t ticket,
                           int64_t key,
                           std::array<uint8_t, REGISTER_SIZE> const &value,
                           int64_t timestamp, call_type type) {
    if (!ctx.add_get_resp(peer_idx, value, timestamp)) {
        return get_finish(ticket, -1, -1, std::array<uint8_t, REGISTER_SIZE>(),
                          false, type);
    } else if (ctx.finished_get_phase(quorum_size())) {
        ctx.finish_get_phase();
        if (ctx.is_unanimous()) {
            return get_finish(ticket, ctx.key(), ctx.timestamp(), ctx.value(),
                              ctx.success(), type);
        }

        if (send_writeback_request_to(ticket, key, ctx.value(), ctx.timestamp(),
                                      type,
                                      ctx.out_of_date_server_idx()) != 0) {
            ERROR("failed to issue writeback request for %ld", ticket);
        }
    }

    return 0;
}

int get_protocol_writeback_round(GetContext &ctx, int64_t ticket,
                                 int64_t timestamp, bool success,
                                 call_type type) {
    if (!ctx.add_put_resp()) {
        return get_finish(ticket, -1, -1, std::array<uint8_t, REGISTER_SIZE>(),
                          false, type);
    } else if (ctx.finished_writeback(quorum_size())) {
        return get_finish(ticket, ctx.key(), ctx.timestamp(), ctx.value(),
                          ctx.success(), type);
    }

    return 0;
}

int get_finish(int64_t ticket, int64_t key, int64_t timestamp,
               ::std::array<uint8_t, REGISTER_SIZE> value, bool success,
               call_type type) {
    g_calls_concluded++;
    switch (type) {
        case call_type::SYNC:
            return get_finish_sync(ticket, key, timestamp, value, success);
        case call_type::ASYNC:
            return get_finish_async(ticket, key, timestamp, value, success);
        case call_type::CALLBACK:
            return get_finish_cb(ticket, key, timestamp, value, success);
    }
}

int get_finish_sync(int64_t, int64_t key, int64_t timestamp,
                    ::std::array<uint8_t, REGISTER_SIZE> value, bool success) {
    g_get_key_result = key;
    g_get_timestamp_result = timestamp;
    std::copy(std::cbegin(value), std::cend(value),
              std::begin(g_get_value_result));
    g_get_success_result = success;
    g_get_sync_ticket = -1;
    return 0;
}
int get_finish_async(int64_t ticket, int64_t key, int64_t timestamp,
                     ::std::array<uint8_t, REGISTER_SIZE> value, bool success) {
    auto it = g_get_ctx_map.find(ticket);
    if (it == g_get_ctx_map.end()) {
        return 0;
    }
    g_get_ctx_map.erase(it);
    g_results_map.emplace(
        ticket,
        std::unique_ptr<result>(
            std::make_unique<one_val_result<std::tuple<
                int64_t, std::array<uint8_t, REGISTER_SIZE>, int64_t, bool>>>(
                one_val_result<
                    std::tuple<int64_t, std::array<uint8_t, REGISTER_SIZE>,
                               int64_t, bool>>(
                    std::make_tuple(key, value, timestamp, success)))));
    return 0;
}

int get_finish_cb(int64_t ticket, int64_t key, int64_t timestamp,
                  ::std::array<uint8_t, REGISTER_SIZE> value, bool success) {
    auto it = g_get_ctx_map.find(ticket);
    if (it == g_get_ctx_map.end()) {
        return 0;
    }
    g_get_ctx_map.erase(it);

    g_get_callback(ticket, key, value, timestamp, success);
    return 0;
}

// put
int put_handler(size_t peer_idx, int64_t ticket, bool success,
                int64_t timestamp) {
    call_type const type = get_call_type(ticket);

    if (operation_finished(ticket, type)) {
        FINE("already finished operation with ticket %ld", ticket);
        return 0;
    }

    if (is_get(ticket, type)) {
        GetContext *ctx = get_get_ctx(ticket, type);
        if (ctx == nullptr) {
            ERROR("Failed to retrieve context for ticket %ld", ticket);
            return -1;
        }

        return get_protocol_writeback_round(*ctx, ticket, timestamp, success,
                                            type);
    } else if (is_put(ticket, type)) {
        PutContext *ctx = get_put_ctx(ticket, type);
        if (ctx == nullptr) {
            ERROR("Failed to retrieve context for ticket %ld", ticket);
            return -1;
        }
        return put_protocol_put_round(*ctx, ticket, timestamp, success, type);
    }

    ERROR(
        "received a put response to an operation which 1: has not finished; 2: "
        "is not a get (writeback); 3: is not a put. [ticket %ld]",
        ticket);
    return -1;
}

PutContext *get_put_ctx(int64_t ticket, call_type type) {
    if (type == call_type::SYNC) {
        return (g_put_sync_ticket == ticket) ? &g_put_sync_ctx : nullptr;
    }

    auto it = g_put_ctx_map.find(ticket);
    if (it == g_put_ctx_map.end()) {
        return nullptr;
    }

    return &it->second;
}

int put_protocol_get_round(PutContext &ctx, int64_t ticket, int64_t key,
                           int64_t timestamp, call_type type) {
    if (!ctx.add_get_resp(timestamp)) {
        return put_finish(ticket, -1, false, type);
    } else if (ctx.finished_get_phase(quorum_size()) &&
               !ctx.started_put_phase()) {
        ctx.finish_get_phase();
        if (send_put_request(ticket, key, ctx.value(), ctx.next_timestamp(),
                             type) != 0) {
            ERROR("failed to issue write request for %ld", ticket);
        }
    }

    return 0;
}

int put_protocol_put_round(PutContext &ctx, int64_t ticket, int64_t timestamp,
                           bool success, call_type type) {
    if (!ctx.add_put_resp()) {
        return put_finish(ticket, -1, false, type);
    } else if (ctx.finished_put_phase(quorum_size())) {
        return put_finish(ticket, ctx.next_timestamp(), ctx.success(), type);
    }

    return 0;
}

int put_finish(int64_t ticket, int64_t timestamp, bool success,
               call_type type) {
    g_calls_concluded++;
    switch (type) {
        case call_type::SYNC:
            return put_finish_sync(ticket, timestamp, success);
        case call_type::ASYNC:
            return put_finish_async(ticket, timestamp, success);
        case call_type::CALLBACK:
            return put_finish_cb(ticket, timestamp, success);
    }
}

int put_finish_sync(int64_t ticket, int64_t timestamp, bool success) {
    g_put_timestamp_result = timestamp;
    g_put_success_result = success;
    g_put_sync_ticket = -1;
    return 0;
}

int put_finish_async(int64_t ticket, int64_t timestamp, bool success) {
    auto it = g_put_ctx_map.find(ticket);
    if (it == g_put_ctx_map.end()) {
        return 0;
    }
    g_put_ctx_map.erase(it);
    g_results_map.emplace(
        ticket, std::unique_ptr<result>(
                    std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                        one_val_result<std::pair<int64_t, bool>>(
                            std::make_pair(timestamp, success)))));
    return 0;
}
int put_finish_cb(int64_t ticket, int64_t timestamp, bool success) {
    auto it = g_put_ctx_map.find(ticket);
    if (it == g_put_ctx_map.end()) {
        return 0;
    }
    g_put_ctx_map.erase(it);
    g_put_callback(ticket, success, timestamp);
    return 0;
}

// proxy get
int proxy_get_handler_sync(int64_t ticket, int64_t key,
                           std::array<uint8_t, REGISTER_SIZE> const &value,
                           int64_t timestamp) {
    g_proxy_get_result = std::make_tuple(key, value, timestamp);
    return 0;
}
int proxy_get_handler_async(int64_t ticket, int64_t key,
                            std::array<uint8_t, REGISTER_SIZE> const &value,
                            int64_t timestamp) {
    g_results_map.emplace(
        ticket,
        std::unique_ptr<result>(
            std::make_unique<one_val_result<std::tuple<
                int64_t, std::array<uint8_t, REGISTER_SIZE>, int64_t>>>(
                one_val_result<std::tuple<
                    int64_t, std::array<uint8_t, REGISTER_SIZE>, int64_t>>(
                    std::make_tuple(key, value, timestamp)))));
    return 0;
}
int proxy_get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                      std::array<uint8_t, REGISTER_SIZE> const &value,
                      int64_t timestamp) {
    g_calls_concluded++;
    switch (ticket % 3) {
        case 0:  // SYNC
            return proxy_get_handler_sync(ticket, key, value, timestamp);
        case 1:  // ASYNC
            return proxy_get_handler_async(ticket, key, value, timestamp);
        case 2:  // CALLBACK
            g_proxy_get_callback(ticket, key, value, timestamp);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// proxy put
int proxy_put_handler_sync(int64_t ticket, bool success, int64_t timestamp) {
    g_proxy_put_result = std::make_pair(timestamp, success);
    return 0;
}
int proxy_put_handler_async(int64_t ticket, bool success, int64_t timestamp) {
    g_results_map.emplace(
        ticket, std::unique_ptr<result>(
                    std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                        one_val_result<std::pair<int64_t, bool>>(
                            std::make_pair(timestamp, success)))));
    return 0;
}
int proxy_put_handler(size_t peer_idx, int64_t ticket, bool success,
                      int64_t timestamp) {
    g_calls_concluded++;
    switch (ticket % 3) {
        case 0:  // SYNC
            return proxy_put_handler_sync(ticket, success, timestamp);
        case 1:  // ASYNC
            return proxy_put_handler_async(ticket, success, timestamp);
        case 2:  // CALLBACK
            g_proxy_put_callback(ticket, success, timestamp);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

// ping
int ping_handler_sync() { return 0; }
int ping_handler_async(int64_t ticket) {
    g_results_map.emplace(ticket, std::make_unique<result>());
    return 0;
}
int ping_handler(size_t peer_idx, int64_t ticket) {
    g_calls_concluded++;
    switch (ticket % 3) {
        case 0:  // SYNC
            return ping_handler_sync();
        case 1:  // ASYNC
            return ping_handler_async(ticket);
        case 2:  // CALLBACK
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
    g_calls_concluded++;
    switch (ticket % 3) {
        case 0:  // SYNC
            return reset_handler_sync();
        case 1:  // ASYNC
            return reset_handler_async(ticket);
        case 2:  // CALLBACK
            g_reset_callback(ticket);
            return 0;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

}  // anonymous namespace
}  // namespace crash
}  // namespace register_sgx
