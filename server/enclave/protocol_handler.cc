#include "protocol_handler.h"

#include "call_map.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"
#include "replicas.h"
#include "setup.h"
#include "teems_generated.h"
#include "user_types.h"

extern teems::CallMap g_call_map;
extern teems::KeyValueStore g_kv_store;

namespace teems {
namespace handler {

namespace {
int get_return_to_client(GetCallContext const &context);
int get_writeback(GetCallContext &context);
int put_return_to_client(PutCallContext const &context);
int put_return_to_client_access_denied(PutCallContext const &context);
int put_do_write(PutCallContext &context);
int put_resp_handler_get_action(GetCallContext &context, bool success,
                                int64_t policy_version, int64_t timestamp);
int put_resp_handler_put_action(PutCallContext &context, bool success,
                                int64_t policy_version, int64_t timestamp);
int do_stabilize(GetCallContext const &context, bool to_outdated);
}  // anonymous namespace

int get_handler(peer &p, int64_t ticket, teems::GetArgs const *args) {
    LOG("get request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, REGISTER_SIZE> value;
    bool stable;
    bool suspicious;
    int64_t timestamp;
    int64_t policy_version;
    ServerPolicy policy;

    bool success = g_kv_store.get(args->key(), &stable, &suspicious,
                                  &policy_version, &timestamp, &policy, &value);

    auto fb_value = teems::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_policy = policy.to_flatbuffers();

    auto get_res =
        teems::CreateGetResult(builder, args->key(), &fb_value, &fb_policy,
                               policy_version, timestamp, stable, suspicious);

    auto result =
        teems::CreateMessage(builder, teems::MessageType_get_resp, ticket,
                             teems::BasicMessage_GetResult, get_res.Union());

    builder.Finish(result);

    return teems::handler_helper::append_message(p, std::move(builder));
}

int get_timestamp_handler(peer &p, int64_t ticket,
                          teems::GetTimestampArgs const *args) {
    LOG("get timestamp request [%ld]: key %ld", ticket, args->key());

    flatbuffers::FlatBufferBuilder builder;

    bool stable;
    bool suspicious;
    int64_t timestamp;
    int64_t policy_version;
    ServerPolicy policy;

    bool success =
        g_kv_store.get_timestamp(args->key(), &stable, &suspicious,
                                 &policy_version, &timestamp, &policy);

    auto fb_policy = policy.to_flatbuffers();
    auto get_timestamp_res =
        teems::CreateGetTimestampResult(builder, args->key(), &fb_policy,
                                        policy_version, timestamp, suspicious);

    auto result = teems::CreateMessage(
        builder, teems::MessageType_get_timestamp_resp, ticket,
        teems::BasicMessage_GetTimestampResult, get_timestamp_res.Union());

    builder.Finish(result);

    return teems::handler_helper::append_message(p, std::move(builder));
}

int put_handler(peer &p, int64_t ticket, teems::PutArgs const *args) {
    LOG("put request [%ld]: key %ld", ticket, args->key());

    int64_t current_timestamp = 0;
    int64_t current_policy_version = 0;
    bool const success =
        g_kv_store.put(args->key(), args->value(), ServerPolicy(args->policy()),
                       args->policy_version(), args->timestamp(),
                       &current_policy_version, &current_timestamp);

    flatbuffers::FlatBufferBuilder builder;

    auto put_res = teems::CreatePutResult(
        builder, success, current_policy_version, current_timestamp);

    auto message =
        teems::CreateMessage(builder, teems::MessageType_put_resp, ticket,
                             teems::BasicMessage_PutResult, put_res.Union());
    builder.Finish(message);

    return teems::handler_helper::append_message(p, std::move(builder));
}

int stabilize_handler(peer &p, int64_t ticket,
                      teems::StabilizeArgs const *args) {
    LOG("stabilize request [%ld]: key %ld", ticket, args->key());
    g_kv_store.stabilize(args->key(), args->policy_version(),
                         args->timestamp());
    return 0;
}

// resp handlers

int get_resp_handler(peer &p, ssize_t peer_idx, int64_t ticket,
                     GetResult const *args) {
    LOG("get response [%ld]: key %ld", ticket, args->key());

    GetCallContext *ctx = g_call_map.get_get_ctx(ticket);
    if (ctx == nullptr) {  // call ended
        return 0;
    }

    std::array<uint8_t, REGISTER_SIZE> value;
    auto const *fb_value = args->value()->data();
    for (size_t idx = 0; idx < REGISTER_SIZE; ++idx) {
        value[idx] = fb_value->Get(idx);
    }

    return get_resp_handler_action(*ctx, peer_idx, ServerPolicy(args->policy()),
                                   value, args->policy_version(),
                                   args->timestamp(), args->stable(),
                                   args->suspicious());
}
int get_resp_handler_action(GetCallContext &context, ssize_t peer_idx,
                            ServerPolicy const &policy,
                            std::array<uint8_t, REGISTER_SIZE> const &value,
                            int64_t policy_version, int64_t timestamp,
                            bool stable, bool suspicious) {
    if (context.get_done()) {
        // get part is done
        return 0;
    }
    auto action = context.add_get_reply(peer_idx, policy_version, timestamp,
                                        stable, suspicious, policy, value);

    switch (action) {
        case GetNextAction::None:
            break;
        case GetNextAction::FallbackPolicyRead:
            // TODO
            INFO("don't know how to do a full policy read");
            break;
        case GetNextAction::Writeback:
            return get_writeback(context);
            break;
        case GetNextAction::StabilizeAndReturnToClient:
            if (do_stabilize(context, false /* to_outdated */) != 0) {
                ERROR("failed to broadcast stabilize");
            }
            return get_return_to_client(context);
            break;
        case GetNextAction::ReturnToClient:
            return get_return_to_client(context);
            break;
    }

    return 0;
}

int get_timestamp_resp_handler(peer &p, int64_t ticket,
                               GetTimestampResult const *args) {
    LOG("get timestamp response [%ld]: key %ld", ticket, args->key());
    PutCallContext *ctx = g_call_map.get_put_ctx(ticket);
    if (ctx == nullptr) {  // call ended
        return 0;
    }

    return get_timestamp_resp_handler_action(
        *ctx, args->policy_version(), args->timestamp(), args->suspicious(),
        ServerPolicy(args->policy()));
}
int get_timestamp_resp_handler_action(PutCallContext &context,
                                      int64_t policy_version, int64_t timestamp,
                                      bool suspicious,
                                      ServerPolicy const &policy) {
    if (context.get_done()) {
        // get part is done
        return 0;
    }

    auto action =
        context.add_get_reply(policy_version, timestamp, suspicious, policy);
    switch (action) {
        case PutNextAction::None:
            return 0;
        case PutNextAction::FallbackPolicyRead:
            // TODO
            INFO("don't know how to do a full policy read");
            return -1;
        case PutNextAction::DoWrite:
            return put_do_write(context);
        default:
            ERROR("there shouldn't be any other actions here");
            return -1;
    }
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
                                           args->policy_version(),
                                           args->timestamp());
    }

    return put_resp_handler_put_action(
        *put_ctx, args->success(), args->policy_version(), args->timestamp());
}

namespace {
int get_return_to_client(GetCallContext const &context) {
    LOG("get [%ld]: returning to client", context.ticket());

    flatbuffers::FlatBufferBuilder builder;
    auto fb_value = teems::Value();
    auto fb_policy = context.policy().to_flatbuffers();

    int64_t policy_version = -1;
    int64_t timestamp = -1;

    if (context.policy().can_get(context.client_id())) {
        auto const &value = context.value();
        flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
            fb_value.mutable_data();
        {
            for (size_t idx = 0; idx < value.size(); ++idx) {
                fb_arr->Mutate(idx, value[idx]);
            }
        }

        policy_version = context.policy_version();
        timestamp = context.timestamp();
    }

    auto get_res = teems::CreateGetResult(
        builder, context.key(), &fb_value, &fb_policy, policy_version,
        timestamp, true /* stable */, false /* suspicious */);

    auto result = teems::CreateMessage(
        builder, teems::MessageType_proxy_get_resp, context.ticket(),
        teems::BasicMessage_GetResult, get_res.Union());

    builder.Finish(result);

    return teems::handler_helper::append_message(*context.client(),
                                                 std::move(builder));
}

int get_writeback(GetCallContext &context) {
    flatbuffers::FlatBufferBuilder builder;

    auto const &value = context.value();
    auto fb_value = teems::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }

    auto fb_policy = context.policy().to_flatbuffers();
    auto put_args =
        teems::CreatePutArgs(builder, context.key(), &fb_value, &fb_policy,
                             context.policy_version(), context.timestamp());

    auto put_request = teems::CreateMessage(
        builder, teems::MessageType_put_req, context.ticket(),
        teems::BasicMessage_PutArgs, put_args.Union());

    builder.Finish(put_request);

    if (context.self_outdated()) {
        int64_t current_policy_version = 0;
        int64_t current_timestamp = 0;
        bool const success =
            g_kv_store.put(context.key(), &fb_value, context.policy(),
                           context.policy_version(), context.timestamp(),
                           &current_policy_version, &current_timestamp);
        if (put_resp_handler_get_action(context, success,
                                        current_policy_version,
                                        current_timestamp) != 0) {
            ERROR("failed to register own action");
        }
    }

    return teems::handler_helper::broadcast_to(context.writeback_indices(),
                                               std::move(builder));
}

int put_return_to_client(PutCallContext const &context) {
    LOG("put [%ld]: returning to client", context.ticket());
    flatbuffers::FlatBufferBuilder builder;

    auto put_res = teems::CreatePutResult(
        builder, true, context.policy_version(), context.next_timestamp());

    auto message = teems::CreateMessage(
        builder, teems::MessageType_proxy_put_resp, context.ticket(),
        teems::BasicMessage_PutResult, put_res.Union());
    builder.Finish(message);

    return teems::handler_helper::append_message(*context.client(),
                                                 std::move(builder));
}

int put_return_to_client_access_denied(PutCallContext const &context) {
    LOG("put [%ld]: returning to client", context.ticket());
    flatbuffers::FlatBufferBuilder builder;

    auto put_res = teems::CreatePutResult(builder, true, -1, -1);

    auto message = teems::CreateMessage(
        builder, teems::MessageType_proxy_put_resp, context.ticket(),
        teems::BasicMessage_PutResult, put_res.Union());
    builder.Finish(message);

    return teems::handler_helper::append_message(*context.client(),
                                                 std::move(builder));
}

int put_do_write(PutCallContext &context) {
    if (!context.policy().can_put(context.client_id())) {
        return put_return_to_client_access_denied(context);
    }

    flatbuffers::FlatBufferBuilder builder;

    auto const &value = context.value();
    auto fb_value = teems::Value();
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *fb_arr =
        fb_value.mutable_data();
    {
        for (size_t idx = 0; idx < value.size(); ++idx) {
            fb_arr->Mutate(idx, value[idx]);
        }
    }
    auto fb_policy = context.policy().to_flatbuffers();

    auto put_args = teems::CreatePutArgs(builder, context.key(), &fb_value,
                                         &fb_policy, context.policy_version(),
                                         context.next_timestamp());

    auto put_request = teems::CreateMessage(
        builder, teems::MessageType_put_req, context.ticket(),
        teems::BasicMessage_PutArgs, put_args.Union());

    builder.Finish(put_request);

    int64_t current_policy_version = 0;
    int64_t current_timestamp = 0;
    bool const success = g_kv_store.put(
        context.key(), &fb_value, context.policy(), context.policy_version(),
        context.next_timestamp(), &current_policy_version, &current_timestamp);
    if (put_resp_handler_put_action(context, success, current_policy_version,
                                    current_timestamp) != 0) {
        ERROR("failed to register own action");
    }

    return teems::handler_helper::broadcast(std::move(builder));
}
int put_resp_handler_get_action(GetCallContext &context, bool success,
                                int64_t policy_version, int64_t timestamp) {
    if (context.call_done()) {
        return 0;  // call finished
    }

    auto action = context.add_writeback_reply();
    switch (action) {
        case GetNextAction::StabilizeAndReturnToClient:
            if (do_stabilize(context, true /* to_outdated */) != 0) {
                ERROR("failed to broadcast stabilize");
            }
            return get_return_to_client(context);
            break;
        case GetNextAction::None:
            return 0;
        default:
            ERROR("invalid action for a writeback reply");
            return -1;
    }
}
int put_resp_handler_put_action(PutCallContext &context, bool success,
                                int64_t policy_version, int64_t timestamp) {
    if (context.call_done()) {
        return 0;  // call finished
    }

    auto action = context.add_put_reply();
    switch (action) {
        case PutNextAction::None:
            return 0;
        case PutNextAction::ReturnToClient:
            return put_return_to_client(context);
        default:
            ERROR("invalid action for a put reply");
            return -1;
    }
}

int do_stabilize(GetCallContext const &context, bool to_outdated) {
    flatbuffers::FlatBufferBuilder builder;

    auto stabilize_args = teems::CreateStabilizeArgs(
        builder, context.key(), context.policy_version(), context.timestamp());

    auto stabilize_request = teems::CreateMessage(
        builder, teems::MessageType_stabilize_req, context.ticket(),
        teems::BasicMessage_StabilizeArgs, stabilize_args.Union());

    builder.Finish(stabilize_request);

    g_kv_store.stabilize(context.key(), context.policy_version(),
                         context.timestamp());
    return teems::handler_helper::broadcast_to(
        to_outdated ? context.all_stabilize_indices()
                    : context.unstable_indices(),
        std::move(builder));
}

}  // anonymous namespace

}  // namespace handler
}  // namespace teems
