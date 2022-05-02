#include "client_handler.h"
#include "call_map.h"
#include "crash_generated.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "replicas.h"
#include "setup.h"
#include "user_types.h"

extern register_sgx::crash::CallMap g_call_map;
extern register_sgx::crash::KeyValueStore g_kv_store;

namespace register_sgx {
namespace crash {
namespace handler {

int client_get_handler(peer &p, int64_t ticket,
                       register_sgx::crash::GetArgs const *args) {
    LOG("client get request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    StoredValue value;
    auto const timestamp = g_kv_store.get(args->key(), &value);

    auto fb_value = register_sgx::crash::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto client_get_res = register_sgx::crash::CreateGetResult(
        builder, args->key(), &fb_value, timestamp);

    auto result = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_resp, ticket,
        register_sgx::crash::BasicMessage_GetResult, client_get_res.Union());

    builder.Finish(result);

    return register_sgx::crash::handler_helper::append_result(
        p, std::move(builder));
}

int client_get_timestamp_handler(
    peer &p, int64_t ticket,
    register_sgx::crash::GetTimestampArgs const *args) {
    LOG("client get request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    auto const timestamp = g_kv_store.get_timestamp(args->key());

    auto client_get_timestamp_res =
        register_sgx::crash::CreateGetTimestampResult(builder, args->key(),
                                                      timestamp);

    auto result = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_get_timestamp_resp, ticket,
        register_sgx::crash::BasicMessage_GetTimestampResult,
        client_get_timestamp_res.Union());

    builder.Finish(result);

    return register_sgx::crash::handler_helper::append_result(
        p, std::move(builder));
}

int client_put_handler(peer &p, int64_t ticket,
                       register_sgx::crash::PutArgs const *args) {
    LOG("client put request [%ld]: key %ld", ticket, args->key());

    int64_t current_timestamp = 0;
    bool const success = g_kv_store.put(args->key(), args->value(),
                                        args->timestamp(), &current_timestamp);

    flatbuffers::FlatBufferBuilder builder;

    auto client_put_res = register_sgx::crash::CreatePutResult(
        builder, success, current_timestamp);

    auto message = register_sgx::crash::CreateMessage(
        builder, register_sgx::crash::MessageType_put_resp, ticket,
        register_sgx::crash::BasicMessage_PutResult, client_put_res.Union());
    builder.Finish(message);

    return register_sgx::crash::handler_helper::append_result(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace crash
}  // namespace register_sgx
