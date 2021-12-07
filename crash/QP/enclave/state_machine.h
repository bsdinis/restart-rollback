/**
 * state_machine.h
 *
 * state machine with wallets
 */

#pragma once

#include <cstdint>
#include <unordered_map>
#include "crash_req_generated.h"

namespace paxos_sgx {
namespace crash {

constexpr int64_t DEFAULT_AMOUNT = 1000;

class StateMachine {
   private:
    ::std::unordered_map<int64_t, int64_t> m_wallets;

   public:
    // return the amount in the account being operated on
    bool execute(OperationArgs const *operation_args, int64_t &account,
                 int64_t &amount);
    int64_t get(int64_t account);
};

}  // namespace crash
}  // namespace paxos_sgx
