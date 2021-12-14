/*
 * op_log.cc
 *
 * The log of operations to execute
 */

#include "op_log.h"
#include "log.h"
#include "persistence.h"
#include "setup.h"

namespace paxos_sgx {
namespace crash {

Operation::Operation(paxos_sgx::crash::OperationArgs const *op) {
    m_account = op->account();
    m_to = op->to();
    m_amount = op->amount();
}

bool Operation::valid() const { return m_account != -1; }

bool Slot::replace_op(Operation op) {
    if (m_op.valid()) {
        return false;
    }
    m_op = op;
    m_accepts = 1;
    return true;
}
void Slot::add_accept() { m_accepts += 1; }
size_t Slot::accepts() const { return m_accepts; }
Operation const *Slot::operation() const { return &m_op; }

size_t OpLog::propose_op(Operation op) {
    if (paxos_sgx::crash::persistence::log_accepted(
            m_log.size(), op.m_account, op.m_amount, op.m_to) == -1) {
        ERROR("failed to log accepted slot %zu", m_log.size());
    }
    m_log.emplace_back(op);
    return m_log.size() - 1;
}
bool OpLog::add_op(size_t slot_n, Operation op) {
    if (paxos_sgx::crash::persistence::log_accepted(
            slot_n, op.m_account, op.m_amount, op.m_to) == -1) {
        ERROR("failed to log accepted slot %zu", slot_n);
    }
    if (slot_n < m_log.size()) {
        return m_log[slot_n].replace_op(op);
    }
    while (slot_n > m_log.size()) {
        m_log.emplace_back();
    }
    m_log.emplace_back(op);

    return true;
}
void OpLog::add_accept(size_t slot_n) {
    m_log[slot_n].add_accept();
    if (slot_n > m_accepted &&
        m_log[slot_n].accepts() > paxos_sgx::crash::setup::quorum_size()) {
        m_accepted = slot_n;
    }
}

void OpLog::executed(size_t slot_n) { m_executed = slot_n; }

bool OpLog::is_accepted(size_t slot_n) const {
    return slot_n < m_log.size() &&
           m_log[slot_n].accepts() >= paxos_sgx::crash::setup::quorum_size();
}
bool OpLog::is_executed(size_t slot_n) const {
    return ((ssize_t)slot_n) <= m_executed;
}
bool OpLog::can_execute(size_t slot_n) const {
    return ((ssize_t)slot_n) > m_executed && is_accepted(slot_n) &&
           (slot_n == 0 || is_executed(slot_n - 1));
}

ssize_t OpLog::execution_cursor() const { return m_executed; }
ssize_t OpLog::accepted_cursor() const { return m_accepted; }
Operation const *OpLog::get_operation(size_t slot_n) const {
    return m_log[slot_n].operation();
}

}  // namespace crash
}  // namespace paxos_sgx
