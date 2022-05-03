#include "ping_handler.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"

namespace register_sgx {
namespace crash {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = register_sgx::crash::CreateEmpty(ping_builder);

    auto request = register_sgx::crash::CreateMessage(
        ping_builder, register_sgx::crash::MessageType_ping_req, ticket,
        register_sgx::crash::BasicMessage_Empty, ping_args.Union());
    ping_builder.Finish(request);

    if (register_sgx::crash::replicas::broadcast_message(
            ping_builder.GetBufferPointer(), ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::crash::CreateEmpty(builder);
    auto result = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_ping_resp, ticket,
        register_sgx::crash::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::crash::handler_helper::append_message(
        p, std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::crash::CreateEmpty(builder);
    auto result = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_ping_resp, ticket,
        register_sgx::crash::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::crash::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
