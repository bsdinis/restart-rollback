#include "smr_handlers.h"

#include "log.h"
#include "op_log.h"
#include "policy.h"
#include "replicas.h"
#include "setup.h"
#include "teems_generated.h"

extern teems::OpLog g_log;

namespace {
int broadcast_accept(int64_t ticket, teems::SmrPropose const *message,
                     bool sus) {
    flatbuffers::FlatBufferBuilder builder;
    auto accept = teems::CreateSmrAccept(builder, message->slot_number(), sus);
    auto msg =
        teems::CreateMessage(builder, teems::MessageType_smr_accept, ticket,
                             teems::BasicMessage_SmrAccept, accept.Union());
    builder.Finish(msg);

    return teems::replicas::broadcast_message(builder.GetBufferPointer(),
                                              builder.GetSize());
}

int broadcast_reject(int64_t ticket, teems::SmrPropose const *message) {
    flatbuffers::FlatBufferBuilder builder;
    auto reject = teems::CreateSmrReject(builder, message->slot_number());
    auto msg =
        teems::CreateMessage(builder, teems::MessageType_smr_reject, ticket,
                             teems::BasicMessage_SmrReject, reject.Union());
    builder.Finish(msg);

    return teems::replicas::broadcast_message(builder.GetBufferPointer(),
                                              builder.GetSize());
}
}  // namespace

namespace teems {
namespace handler {

int smr_propose_handler(peer &p, int64_t ticket,
                        teems::SmrPropose const *message) {
    LOG("replica propose request [%ld]: slot %ld", ticket,
        message->slot_number());

    if (g_log.add_op(message->slot_number(),
                     Operation(message->key(), message->policy_read(),
                               ServerPolicy(message->policy())),
                     message->suspicious())) {
        replicas::add_accept(message->slot_number(),
                             teems::setup::is_suspicious());
        return broadcast_accept(ticket, message, teems::setup::is_suspicious());
    }

    return broadcast_reject(ticket, message);
}

int smr_accept_handler(peer &p, int64_t ticket,
                       teems::SmrAccept const *message) {
    LOG("replica accept request [%ld]: slot %ld", ticket,
        message->slot_number());

    teems::replicas::add_accept(message->slot_number(), message->suspicious());
    return 0;
}

int smr_reject_handler(peer &p, int64_t ticket,
                       teems::SmrReject const *message) {
    LOG("replica reject request [%ld]: slot %ld", ticket,
        message->slot_number());
    KILL("Without a view change, there should never be a rejection");
    return 0;
}

}  // namespace handler
}  // namespace teems
