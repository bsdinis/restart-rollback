#include "protocol_handler.h"

#include "byzantine_generated.h"
#include "call_map.h"
#include "digital_signature.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "replicas.h"
#include "setup.h"
#include "user_types.h"

extern register_sgx::byzantine::CallMap g_call_map;
extern register_sgx::byzantine::KeyValueStore g_kv_store;

namespace register_sgx {
namespace byzantine {
namespace handler {

namespace {
int get_return_to_client(GetCallContext const &context);
int get_writeback(GetCallContext &context);
int put_return_to_client(PutCallContext const &context);
int put_do_write(PutCallContext &context);
int put_resp_handler_get_action(GetCallContext &context, bool success,
                                int64_t timestamp);
int put_resp_handler_put_action(PutCallContext &context, bool success,
                                int64_t timestamp);
}  // anonymous namespace

int get_handler(peer &p, int64_t ticket,
                register_sgx::byzantine::GetArgs const *args) {
    LOG("get request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, REGISTER_SIZE> value;
    std::vector<uint8_t> signature;
    auto const timestamp = g_kv_store.get(args->key(), &value, signature);

    auto data_value = register_sgx::byzantine::DataValue();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        data_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_value = register_sgx::byzantine::CreateValue(
        builder, args->key(), &data_value, timestamp);
    auto signed_value = register_sgx::byzantine::CreateSignedValueDirect(
        builder, fb_value, &signature);
    auto get_res =
        register_sgx::byzantine::CreateGetResult(builder, signed_value);

    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_get_resp, ticket,
        register_sgx::byzantine::BasicMessage_GetResult, get_res.Union());

    builder.Finish(result);

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

int get_timestamp_handler(
    peer &p, int64_t ticket,
    register_sgx::byzantine::GetTimestampArgs const *args) {
    LOG("get timestamp request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, REGISTER_SIZE> value;
    std::vector<uint8_t> signature;
    auto const timestamp = g_kv_store.get(args->key(), &value, signature);

    auto data_value = register_sgx::byzantine::DataValue();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        data_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_value = register_sgx::byzantine::CreateValue(
        builder, args->key(), &data_value, timestamp);
    auto signed_value = register_sgx::byzantine::CreateSignedValueDirect(
        builder, fb_value, &signature);
    auto get_timestamp_res = register_sgx::byzantine::CreateGetTimestampResult(
        builder, signed_value);

    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_get_timestamp_resp,
        ticket, register_sgx::byzantine::BasicMessage_GetTimestampResult,
        get_timestamp_res.Union());

    builder.Finish(result);

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

int put_handler(peer &p, int64_t ticket,
                register_sgx::byzantine::PutArgs const *args) {
    LOG("put request [%ld]: key %ld", ticket,
        args->signed_value()->value()->key());

    bool authentic = false;
    if (args->signed_value()->value()->timestamp() > 0 &&
        (verify_value(args->signed_value(), &authentic) != 0 || !authentic)) {
        ERROR("failed to authenticate value");
        return -1;
    }

    std::vector<uint8_t> signature;
    signature.reserve(args->signed_value()->signature()->size());
    std::copy(args->signed_value()->signature()->cbegin(),
              args->signed_value()->signature()->cend(),
              std::back_inserter(signature));

    int64_t current_timestamp = 0;
    bool const success =
        g_kv_store.put(args->signed_value()->value()->key(),
                       args->signed_value()->value()->data_value(),
                       args->signed_value()->value()->timestamp(), signature,
                       &current_timestamp);

    flatbuffers::FlatBufferBuilder builder;

    auto put_res = register_sgx::byzantine::CreatePutResult(builder, success,
                                                            current_timestamp);

    auto message = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_put_resp, ticket,
        register_sgx::byzantine::BasicMessage_PutResult, put_res.Union());
    builder.Finish(message);

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

// resp handlers

int get_resp_handler(peer &p, size_t peer_idx, int64_t ticket,
                     GetResult const *args) {
    LOG("get response [%ld]: key %ld", ticket,
        args->signed_value()->value()->key());

    bool authentic = false;
    if (args->signed_value()->value()->timestamp() > 0 &&
        (verify_value(args->signed_value(), &authentic) != 0 || !authentic)) {
        ERROR("failed to authenticate value");
        return -1;
    }

    GetCallContext *ctx = g_call_map.get_get_ctx(ticket);
    if (ctx == nullptr) {  // call ended
        return 0;
    }

    std::array<uint8_t, REGISTER_SIZE> value;
    auto const *fb_value = args->signed_value()->value()->data_value()->data();
    for (size_t idx = 0; idx < REGISTER_SIZE; ++idx) {
        value[idx] = fb_value->Get(idx);
    }

    std::vector<uint8_t> signature;
    signature.reserve(args->signed_value()->signature()->size());
    std::copy(args->signed_value()->signature()->cbegin(),
              args->signed_value()->signature()->cend(),
              std::back_inserter(signature));

    return get_resp_handler_action(*ctx, peer_idx, value,
                                   args->signed_value()->value()->timestamp(),
                                   std::move(signature));
}
int get_resp_handler_action(GetCallContext &context, size_t peer_idx,
                            std::array<uint8_t, REGISTER_SIZE> const &value,
                            int64_t timestamp,
                            std::vector<uint8_t> &&signature) {
    if (context.get_done()) {
        // get part is done
        return 0;
    }
    context.add_get_reply(peer_idx, timestamp, value, std::move(signature));

    if (context.get_done()) {
        context.finish_get_phase();
        if (context.call_done()) {
            return get_return_to_client(context);
        } else {
            return get_writeback(context);
        }
    }

    return 0;
}

int get_timestamp_resp_handler(peer &p, int64_t ticket,
                               GetTimestampResult const *args) {
    LOG("get timestamp response [%ld]: key %ld", ticket,
        args->signed_value()->value()->key());

    bool authentic = false;
    if (args->signed_value()->value()->timestamp() > 0 &&
        (verify_value(args->signed_value(), &authentic) != 0 || !authentic)) {
        ERROR("failed to authenticate value");
        return -1;
    }

    PutCallContext *ctx = g_call_map.get_put_ctx(ticket);
    if (ctx == nullptr) {  // call ended
        return 0;
    }

    return get_timestamp_resp_handler_action(
        *ctx, args->signed_value()->value()->timestamp());
}
int get_timestamp_resp_handler_action(PutCallContext &context,
                                      int64_t timestamp) {
    if (context.get_done()) {
        // get part is done
        return 0;
    }

    context.add_get_reply(timestamp);

    if (context.get_done()) {
        return put_do_write(context);
    }

    return 0;
}

int put_resp_handler(peer &p, int64_t ticket, PutResult const *args) {
    LOG("put response [%ld]", ticket);
    GetCallContext *get_ctx = g_call_map.get_get_ctx(ticket);
    PutCallContext *put_ctx = g_call_map.get_put_ctx(ticket);
    if (get_ctx == nullptr && put_ctx == nullptr) {  // call ended
        return 0;
    } else if (get_ctx != nullptr && put_ctx != nullptr) {
        ERROR("cannot have two calls with the same ticket: %ld", ticket);
        return -1;
    }

    if (get_ctx != nullptr) {
        return put_resp_handler_get_action(*get_ctx, args->success(),
                                           args->timestamp());
    }

    return put_resp_handler_put_action(*put_ctx, args->success(),
                                       args->timestamp());
}

namespace {
int get_return_to_client(GetCallContext const &context) {
    LOG("get [%ld]: returning to client (%ld)", context.ticket());
    flatbuffers::FlatBufferBuilder builder;

    auto const &value = context.value();
    auto data_value = register_sgx::byzantine::DataValue();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        data_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_value = register_sgx::byzantine::CreateValue(
        builder, context.key(), &data_value, context.timestamp());
    auto signed_value = register_sgx::byzantine::CreateSignedValueDirect(
        builder, fb_value, &context.signature());
    auto get_res =
        register_sgx::byzantine::CreateGetResult(builder, signed_value);

    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_proxy_get_resp,
        context.ticket(), register_sgx::byzantine::BasicMessage_GetResult,
        get_res.Union());

    builder.Finish(result);

    return register_sgx::byzantine::handler_helper::append_message(
        *context.client(), std::move(builder));
}

int get_writeback(GetCallContext &context) {
    flatbuffers::FlatBufferBuilder builder;

    auto const &value = context.value();
    auto data_value = register_sgx::byzantine::DataValue();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        data_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_value = register_sgx::byzantine::CreateValue(
        builder, context.key(), &data_value, context.timestamp());
    auto signed_value = register_sgx::byzantine::CreateSignedValueDirect(
        builder, fb_value, &context.signature());

    auto put_args =
        register_sgx::byzantine::CreatePutArgs(builder, signed_value);

    auto put_request = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_put_req, context.ticket(),
        register_sgx::byzantine::BasicMessage_PutArgs, put_args.Union());

    builder.Finish(put_request);

    if (context.self_outdated()) {
        int64_t current_timestamp = 0;
        bool const success =
            g_kv_store.put(context.key(), &data_value, context.timestamp(),
                           context.signature(), &current_timestamp);
        if (put_resp_handler_get_action(context, success, current_timestamp) !=
            0) {
            ERROR("failed to register own action");
        }
    }

    return register_sgx::byzantine::handler_helper::broadcast_to(
        context.writeback_indices(), std::move(builder));
}

int put_return_to_client(PutCallContext const &context) {
    LOG("put [%ld]: returning to client", context.ticket());
    flatbuffers::FlatBufferBuilder builder;

    auto put_res = register_sgx::byzantine::CreatePutResult(
        builder, true, context.next_timestamp());

    auto message = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_proxy_put_resp,
        context.ticket(), register_sgx::byzantine::BasicMessage_PutResult,
        put_res.Union());
    builder.Finish(message);

    return register_sgx::byzantine::handler_helper::append_message(
        *context.client(), std::move(builder));
}

int put_do_write(PutCallContext &context) {
    if (context.sign() != 0) {
        ERROR("failed to sign value");
        return -1;
    }

    flatbuffers::FlatBufferBuilder builder;

    auto const &value = context.value();
    auto data_value = register_sgx::byzantine::DataValue();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        data_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_value = register_sgx::byzantine::CreateValue(
        builder, context.key(), &data_value, context.next_timestamp());
    auto signed_value = register_sgx::byzantine::CreateSignedValueDirect(
        builder, fb_value, &context.signature());
    auto put_args =
        register_sgx::byzantine::CreatePutArgs(builder, signed_value);

    auto put_request = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_put_req, context.ticket(),
        register_sgx::byzantine::BasicMessage_PutArgs, put_args.Union());

    builder.Finish(put_request);

    int64_t current_timestamp = 0;
    bool const success =
        g_kv_store.put(context.key(), &data_value, context.next_timestamp(),
                       context.signature(), &current_timestamp);
    if (put_resp_handler_put_action(context, success, current_timestamp) != 0) {
        ERROR("failed to register own action");
    }

    return register_sgx::byzantine::handler_helper::broadcast(
        std::move(builder));
}
int put_resp_handler_get_action(GetCallContext &context, bool success,
                                int64_t timestamp) {
    if (context.call_done()) {
        return 0;  // call finished
    }

    context.add_writeback_reply();
    if (context.call_done()) {
        return get_return_to_client(context);
    }
    return 0;
}
int put_resp_handler_put_action(PutCallContext &context, bool success,
                                int64_t timestamp) {
    if (context.call_done()) {
        return 0;  // call finished
    }

    context.add_put_reply();
    if (context.call_done()) {
        return put_return_to_client(context);
    }
    return 0;
}

}  // anonymous namespace

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
