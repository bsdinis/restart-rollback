#include "reset_handler.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"

extern teems::KeyValueStore g_kv_store;

namespace teems {
namespace handler {

int reset_handler(peer &p, int64_t ticket) {
    LOG("reset request [%ld]", ticket);

    g_kv_store.reset();

    flatbuffers::FlatBufferBuilder builder;
    auto reset_res = teems::CreateEmpty(builder);
    auto result =
        teems::CreateMessage(builder, teems::MessageType_reset_resp, ticket,
                             teems::BasicMessage_Empty, reset_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    return teems::handler_helper::append_message(p, std::move(builder));
}

}  // namespace handler
}  // namespace teems
