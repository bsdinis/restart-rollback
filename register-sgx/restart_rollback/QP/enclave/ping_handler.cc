#include "ping_handler.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"
#include "restart_rollback_generated.h"

namespace register_sgx {
namespace restart_rollback {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = register_sgx::restart_rollback::CreateEmpty(ping_builder);

    auto request = register_sgx::restart_rollback::CreateMessage(
        ping_builder, register_sgx::restart_rollback::MessageType_ping_req,
        ticket, register_sgx::restart_rollback::BasicMessage_Empty,
        ping_args.Union());
    ping_builder.Finish(request);

    if (register_sgx::restart_rollback::replicas::broadcast_message(
            ping_builder.GetBufferPointer(), ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::restart_rollback::CreateEmpty(builder);
    auto result = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_ping_resp, ticket,
        register_sgx::restart_rollback::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::restart_rollback::handler_helper::append_message(
        p, std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    LOG("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = register_sgx::restart_rollback::CreateEmpty(builder);
    auto result = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_ping_resp, ticket,
        register_sgx::restart_rollback::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return register_sgx::restart_rollback::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
