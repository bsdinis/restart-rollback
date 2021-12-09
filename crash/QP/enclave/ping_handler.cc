#include "ping_handler.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "replicas.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int client_ping_handler(peer &p, int64_t ticket) {
    INFO("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder ping_builder;
    auto ping_args = paxos_sgx::crash::CreateEmpty(ping_builder);

    auto request = paxos_sgx::crash::CreateMessage(
        ping_builder, paxos_sgx::crash::MessageType_ping_req, ticket,
        paxos_sgx::crash::BasicMessage_Empty, ping_args.Union());
    ping_builder.Finish(request);

    if (paxos_sgx::crash::replicas::broadcast_message(
            ping_builder.GetBufferPointer(), ping_builder.GetSize()) == -1) {
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::crash::CreateEmpty(builder);
    auto result = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_ping_resp, ticket,
        paxos_sgx::crash::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

int replica_ping_handler(peer &p, int64_t ticket) {
    INFO("ping request [%ld]", ticket);

    flatbuffers::FlatBufferBuilder builder;
    auto ping_res = paxos_sgx::crash::CreateEmpty(builder);
    auto result = paxos_sgx::crash::CreateMessage(
        builder, paxos_sgx::crash::MessageType_ping_resp, ticket,
        paxos_sgx::crash::BasicMessage_Empty, ping_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
