/**
 * state_machine.h
 *
 * state machine with wallets
 */

#pragma once

#include <cstdint>
#include <unordered_map>
#include "op_log.h"

namespace paxos_sgx {
namespace restart_rollback {

constexpr int64_t DEFAULT_AMOUNT = 1000;

class StateMachine {
   private:
    ::std::unordered_map<int64_t, int64_t> m_wallets;

   public:
    // return the amount in the account being operated on
    bool execute(Operation const *operation, int64_t &account, int64_t &amount);
    int64_t get(int64_t account);
    void reset();
};

}  // namespace restart_rollback
}  // namespace paxos_sgx
