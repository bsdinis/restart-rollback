#include "proxy_handler.h"

#include "call_map.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "protocol_handler.h"
#include "replicas.h"
#include "teems_generated.h"
#include "user_types.h"

extern teems::CallMap g_call_map;
extern teems::KeyValueStore g_kv_store;

namespace teems {
namespace handler {

int proxy_get_handler(peer &p, int64_t ticket, GetArgs const *args) {
    LOG("proxy get request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_get_call(&p, ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;
    auto get_args = teems::CreateGetArgs(builder, args->key());
    auto get_req =
        teems::CreateMessage(builder, teems::MessageType_get_req, ticket,
                             teems::BasicMessage_GetArgs, get_args.Union());

    builder.Finish(get_req);

    if (teems::handler_helper::broadcast(std::move(builder)) != 0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    std::array<uint8_t, REGISTER_SIZE> value;
    bool stable;
    bool suspicious;
    int64_t timestamp;
    int64_t policy_version;
    ServerPolicy policy;

    bool success = g_kv_store.get(args->key(), &stable, &suspicious,
                                  &policy_version, &timestamp, &policy, &value);
    return get_resp_handler_action(*ctx, -1, policy, value, policy_version,
                                   timestamp, stable, suspicious);
}

int proxy_put_handler(peer &p, int64_t ticket, ProxyPutArgs const *args) {
    LOG("proxy put request [%ld]: key %ld", ticket, args->key());

    auto ctx = g_call_map.add_put_call(&p, ticket, args->key(),
                                       args->client_id(), args->value());

    flatbuffers::FlatBufferBuilder builder;
    auto get_timestamp_args =
        teems::CreateGetTimestampArgs(builder, args->key());
    auto get_timestamp_req = teems::CreateMessage(
        builder, teems::MessageType_get_timestamp_req, ticket,
        teems::BasicMessage_GetTimestampArgs, get_timestamp_args.Union());

    builder.Finish(get_timestamp_req);

    if (teems::handler_helper::broadcast(std::move(builder)) != 0) {
        ERROR("failed to broadcast get request");
        return -1;
    }

    bool stable;
    bool suspicious;
    int64_t timestamp;
    int64_t policy_version;

    g_kv_store.get_timestamp(args->key(), &stable, &suspicious, &policy_version,
                             &timestamp);
    return get_timestamp_resp_handler_action(*ctx, policy_version, timestamp,
                                             suspicious);
}

}  // namespace handler
}  // namespace teems
