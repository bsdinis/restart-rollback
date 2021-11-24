#include "sum_handler.h"

#include "basicQP_resp_generated.h"
#include "log.h"

namespace epidemics {
namespace basicQP {
namespace handler {

int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec) {
    LOG("sum request [%ld]: vector with %zu els", ticket, vec.size());
    int64_t sum = 0;
    for (const auto &el : vec) sum += el;

    flatbuffers::FlatBufferBuilder builder;
    auto sum_res = epidemics::basicQP::CreateSumResult(builder, sum);
    auto result = epidemics::basicQP::CreateBasicResponse(
        builder, epidemics::basicQP::ReqType_sum, ticket,
        epidemics::basicQP::Result_SumResult, sum_res.Union());
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
}  // namespace basicQP
}  // namespace epidemics
