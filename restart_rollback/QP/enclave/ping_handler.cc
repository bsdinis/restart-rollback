#include "ping_handler.h"
#include "restart_rollback_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    INFO("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = paxos_sgx::restart_rollback::CreateEmpty(ping_builder);

    auto request = paxos_sgx::restart_rollback::CreateMessage(
        ping_builder, paxos_sgx::restart_rollback::MessageType_ping_req, ticket,
        paxos_sgx::restart_rollback::BasicMessage_Empty, ping_args.Union());
    ping_builder.Finish(request);

    if (paxos_sgx::restart_rollback::replicas::broadcast_message(
            ping_builder.GetBufferPointer(), ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::restart_rollback::CreateEmpty(builder);
    auto result = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_ping_resp, ticket,
        paxos_sgx::restart_rollback::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return paxos_sgx::restart_rollback::handler_helper::append_result(p,
                                                           std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    INFO("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::restart_rollback::CreateEmpty(builder);
    auto result = paxos_sgx::restart_rollback::CreateMessage(
        builder, paxos_sgx::restart_rollback::MessageType_ping_resp, ticket,
        paxos_sgx::restart_rollback::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return paxos_sgx::restart_rollback::handler_helper::append_result(p,
                                                           std::move(builder));
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
