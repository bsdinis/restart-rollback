#include "proxy_handler.h"

#include "call_map.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "protocol_handler.h"
#include "replicas.h"
#include "restart_rollback_generated.h"
#include "user_types.h"

extern register_sgx::restart_rollback::CallMap g_call_map;
extern register_sgx::restart_rollback::KeyValueStore g_kv_store;

namespace register_sgx {
namespace restart_rollback {
namespace handler {

int proxy_get_handler(peer &p, int64_t ticket, GetArgs const *args) {
    LOG("proxy get request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_get_call(&p, ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;
    auto get_args =
        register_sgx::restart_rollback::CreateGetArgs(builder, args->key());
    auto get_req = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_get_req, ticket,
        register_sgx::restart_rollback::BasicMessage_GetArgs, get_args.Union());

    builder.Finish(get_req);

    if (register_sgx::restart_rollback::handler_helper::broadcast(
            std::move(builder)) != 0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    std::array<uint8_t, REGISTER_SIZE> value;
    bool stable = false;
    bool suspicious = false;
    auto const timestamp =
        g_kv_store.get(args->key(), &stable, &suspicious, &value);
    return get_resp_handler_action(*ctx, -1, value, timestamp, stable,
                                   suspicious);
}

int proxy_put_handler(peer &p, int64_t ticket, ProxyPutArgs const *args) {
    LOG("proxy put request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_put_call(&p, ticket, args->key(),
                                       args->client_id(), args->value());

    flatbuffers::FlatBufferBuilder builder;
    auto get_timestamp_args =
        register_sgx::restart_rollback::CreateGetTimestampArgs(builder,
                                                               args->key());
    auto get_timestamp_req = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_get_timestamp_req,
        ticket, register_sgx::restart_rollback::BasicMessage_GetTimestampArgs,
        get_timestamp_args.Union());

    builder.Finish(get_timestamp_req);

    if (register_sgx::restart_rollback::handler_helper::broadcast(
            std::move(builder)) != 0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    bool suspicious = false;
    auto const timestamp = g_kv_store.get_timestamp(args->key(), &suspicious);
    return get_timestamp_resp_handler_action(*ctx, timestamp, suspicious);
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
