#include "protocol_helpers.h"

#include "log.h"
#include "metadata.h"
#include "network.h"
#include "result.h"
#include "teems_config.h"
#include "teems_generated.h"

#include <algorithm>
#include <unordered_map>

namespace teems {

extern int32_t g_client_id;

extern ::std::vector<peer> g_servers;
extern size_t g_calls_issued;
extern size_t g_calls_concluded;

extern std::function<void(int64_t)> g_ping_callback;
extern std::function<void(int64_t)> g_reset_callback;

// global variables to channel the results of synchronous calls to

// async results
::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;

namespace {
int greeting_handler(size_t peer_idx, int32_t id);

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
int64_t send_ping_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

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

    g_calls_issued++;
    return ticket;
}

int64_t send_reset_request(peer &server, call_type type) {
    int64_t const ticket = gen_ticket(type);

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
                    response->message_as_GetResult()->timestamp());
                break;
            case teems::MessageType_proxy_put_resp:
                FINE("proxy put response [ticket %ld]", response->ticket());
                metadata_put_handler(
                    idx, response->ticket(),
                    response->message_as_PutResult()->success(),
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

bool has_result(int64_t ticket) {
    return g_results_map.find(ticket) != std::end(g_results_map);
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
}  // namespace teems
