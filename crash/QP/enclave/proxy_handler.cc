#include "proxy_handler.h"

#include "call_map.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "protocol_handler.h"
#include "replicas.h"
#include "user_types.h"

extern register_sgx::crash::CallMap g_call_map;
extern register_sgx::crash::KeyValueStore g_kv_store;

namespace register_sgx {
namespace crash {
namespace handler {

int proxy_get_handler(peer &p, int64_t ticket, GetArgs const *args) {
    LOG("proxy get request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_get_call(&p, ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;
    auto get_args = register_sgx::crash::CreateGetArgs(builder, args->key());
    auto get_req = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_req, ticket,
        register_sgx::crash::BasicMessage_GetArgs, get_args.Union());

    builder.Finish(get_req);

    if (register_sgx::crash::handler_helper::broadcast(std::move(builder)) !=
        0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    std::array<uint8_t, REGISTER_SIZE> value;
    auto const timestamp = g_kv_store.get(args->key(), &value);
    return get_resp_handler_action(*ctx, -1, value, timestamp);
}

int proxy_put_handler(peer &p, int64_t ticket, ProxyPutArgs const *args) {
    LOG("proxy put request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_put_call(&p, ticket, args->key(),
                                       args->client_id(), args->value());

    flatbuffers::FlatBufferBuilder builder;
    auto get_timestamp_args =
        register_sgx::crash::CreateGetTimestampArgs(builder, args->key());
    auto get_timestamp_req = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_timestamp_req, ticket,
        register_sgx::crash::BasicMessage_GetTimestampArgs,
        get_timestamp_args.Union());

    builder.Finish(get_timestamp_req);

    if (register_sgx::crash::handler_helper::broadcast(std::move(builder)) !=
        0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    auto const timestamp = g_kv_store.get_timestamp(args->key());
    return get_timestamp_resp_handler_action(*ctx, timestamp);
}

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
