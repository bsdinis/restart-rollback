#include "reset_handler.h"
#include "handler_helpers.h"
#include "kv_store.h"
#include "log.h"

extern register_sgx::restart_rollback::KeyValueStore g_kv_store;

namespace register_sgx {
namespace restart_rollback {
namespace handler {

int reset_handler(peer &p, int64_t ticket) {
    LOG("reset request [%ld]", ticket);

    g_kv_store.reset();

    flatbuffers::FlatBufferBuilder builder;
    auto reset_res = register_sgx::restart_rollback::CreateEmpty(builder);
    auto result = register_sgx::restart_rollback::CreateMessage(
        builder, register_sgx::restart_rollback::MessageType_reset_resp, ticket,
        register_sgx::restart_rollback::BasicMessage_Empty, reset_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    return register_sgx::restart_rollback::handler_helper::append_message(
        p, std::move(builder));
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace register_sgx
