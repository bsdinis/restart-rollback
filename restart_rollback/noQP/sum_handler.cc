#include "sum_handler.h"

#include "restart_rollback_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace restart_rollback {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec) {
    LOG("sum request [%ld]: vector with %u els", ticket, vec.size());
    int64_t sum = 0;
    for (const auto &el : vec) sum += el;

    flatbuffers::FlatBufferBuilder builder;
    auto sum_res = paxos_sgx::restart_rollback::CreateSumResult(builder, sum);
    auto result = paxos_sgx::restart_rollback::CreateBasicResponse(
        builder, paxos_sgx::restart_rollback::ReqType_sum, ticket,
        paxos_sgx::restart_rollback::Result_SumResult, sum_res.Union());
    builder.Finish(result);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    FINE("response has %zu B", size);
    if (p.append(&size, 1) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    } else if (p.append(payload, size) == -1) {
        ERROR("failed to prepare message to send");
        return -1;
    }
    return 0;
}

}  // namespace handler
}  // namespace restart_rollback
}  // namespace paxos_sgx
