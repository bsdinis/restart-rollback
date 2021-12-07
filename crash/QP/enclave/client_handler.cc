
#include "client_handler.h"
#include "crash_resp_generated.h"
#include "handler_helpers.h"
#include "log.h"
#include "state_machine.h"

extern paxos_sgx::crash::StateMachine g_state_machine;

namespace paxos_sgx {
namespace crash {
namespace handler {

int client_fast_get_handler(peer &p, int64_t ticket,
                            paxos_sgx::crash::ClientFastGetArgs const *args) {
    LOG("client fast get request [%ld]: account %ld", ticket, args->account());

    flatbuffers::FlatBufferBuilder builder;
    auto client_fast_get_res = paxos_sgx::crash::CreateClientFastGetResult(
        builder, args->account(), g_state_machine.get(args->account()), true);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_client_fast_get, ticket,
        paxos_sgx::crash::Result_ClientFastGetResult,
        client_fast_get_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

int client_operation_handler(peer &p, int64_t ticket,
                             paxos_sgx::crash::OperationArgs const *args) {
    LOG("client operation request [%ld]", ticket);

    int64_t account;
    int64_t amount;
    bool success = g_state_machine.execute(args, account, amount);

    flatbuffers::FlatBufferBuilder builder;
    auto client_operation_res = paxos_sgx::crash::CreateClientOperationResult(
        builder, account, amount, success);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_client_operation, ticket,
        paxos_sgx::crash::Result_ClientOperationResult,
        client_operation_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
                                                           std::move(builder));
}

/*
int sum_handler(peer &p, int64_t ticket,
                flatbuffers::Vector<int64_t> const &vec) {
    LOG("sum request [%ld]: vector with %zu els", ticket, vec.size());
    int64_t sum = 0;
    for (const auto &el : vec) sum += el;

    flatbuffers::FlatBufferBuilder builder;
    auto sum_res = paxos_sgx::crash::CreateSumResult(builder, sum);
    auto result = paxos_sgx::crash::CreateBasicResponse(
        builder, paxos_sgx::crash::ReqType_sum, ticket,
        paxos_sgx::crash::Result_SumResult, sum_res.Union());
    builder.Finish(result);

    return paxos_sgx::crash::handler_helper::append_result(p,
std::move(builder));
}
*/

}  // namespace handler
}  // namespace crash
}  // namespace paxos_sgx
