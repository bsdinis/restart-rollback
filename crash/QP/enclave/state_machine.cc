#include "state_machine.h"
#include "log.h"

namespace paxos_sgx {
namespace crash {

int64_t StateMachine::get(int64_t account) {
    auto account_it = this->m_wallets.find(account);
    if (account_it == this->m_wallets.end()) {
        m_wallets.insert({account, DEFAULT_AMOUNT});
        return DEFAULT_AMOUNT;
    }

    return account_it->second;
}

bool StateMachine::execute(OperationArgs const *operation, int64_t &account,
                           int64_t &res_amount) {
    account = operation->account();
    int64_t cur_amount = this->get(account);
    auto amount = operation->amount();

    // actual transfer
    if (amount > 0) {
        auto to = operation->to();
        this->get(to);

        if (cur_amount >= amount) {
            this->m_wallets[account] -= amount;
            this->m_wallets[to] += amount;
            res_amount = cur_amount - amount;
            return true;
        }

        res_amount = cur_amount;
        return false;
    }

    // simple get
    res_amount = cur_amount;
    return true;
}

void StateMachine::reset() {
    this->m_wallets = ::std::unordered_map<int64_t, int64_t>();
}

}  // namespace crash
}  // namespace paxos_sgx
