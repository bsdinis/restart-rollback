#include "ping_handler.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"
#include "teems_generated.h"

namespace teems {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = teems::CreateEmpty(ping_builder);

    auto request =
        teems::CreateMessage(ping_builder, teems::MessageType_ping_req, ticket,
                             teems::BasicMessage_Empty, ping_args.Union());
    ping_builder.Finish(request);

    if (teems::replicas::broadcast_message(ping_builder.GetBufferPointer(),
                                           ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = teems::CreateEmpty(builder);
    auto result =
        teems::CreateMessage(builder, teems::MessageType_ping_resp, ticket,
                             teems::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return teems::handler_helper::append_message(p, std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = teems::CreateEmpty(builder);
    auto result =
        teems::CreateMessage(builder, teems::MessageType_ping_resp, ticket,
                             teems::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return teems::handler_helper::append_message(p, std::move(builder));
}

}  // namespace handler
}  // namespace teems
