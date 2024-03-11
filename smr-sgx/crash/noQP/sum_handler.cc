#include "sum_handler.h"

#include "crash_resp_generated.h"
#include "log.h"

namespace paxos_sgx {
namespace crash {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec) {
    LOG("sum request [%ld]: vector with %u els", ticket, vec.size());
    int64_t sum = 0;
    for (const auto &el : vec) sum += el;

    flatbuffers::FlatBufferBuilder builder;
    auto sum_res = paxos_sgx::crash::CreateSumResult(builder, sum);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_sum, ticket,
        paxos_sgx::crash::Result_SumResult, sum_res.Union());
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
}  // namespace crash
}  // namespace paxos_sgx
