#include "reset_handler.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"

extern register_sgx::byzantine::KeyValueStore g_kv_store;

namespace register_sgx {
namespace byzantine {
namespace handler {

int reset_handler(peer &p, int64_t ticket) {
    LOG("reset request [%ld]", ticket);

    g_kv_store.reset();

    flatbuffers::FlatBufferBuilder builder;
    auto reset_res = register_sgx::byzantine::CreateEmpty(builder);
    auto result = register_sgx::byzantine::CreateMessage(
        builder, register_sgx::byzantine::MessageType_reset_resp, ticket,
        register_sgx::byzantine::BasicMessage_Empty, reset_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    return register_sgx::byzantine::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace byzantine
}  // namespace register_sgx
