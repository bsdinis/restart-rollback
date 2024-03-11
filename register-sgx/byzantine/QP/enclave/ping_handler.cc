#include "ping_handler.h"
#include "byzantine_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"

namespace register_sgx {
namespace byzantine {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = register_sgx::byzantine::CreateEmpty(ping_builder);

    auto request = register_sgx::byzantine::CreateMessage(
        ping_builder, register_sgx::byzantine::MessageType_ping_req, ticket,
        register_sgx::byzantine::BasicMessage_Empty, ping_args.Union());
    ping_builder.Finish(request);

    if (register_sgx::byzantine::replicas::broadcast_message(
            ping_builder.GetBufferPointer(), ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::byzantine::CreateEmpty(builder);
    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_ping_resp, ticket,
        register_sgx::byzantine::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::byzantine::CreateEmpty(builder);
    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_ping_resp, ticket,
        register_sgx::byzantine::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
