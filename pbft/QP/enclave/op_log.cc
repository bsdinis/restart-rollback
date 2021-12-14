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
namespace pbft {

Operation::Operation(paxos_sgx::pbft::OperationArgs const *op) {
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
    m_prepares = 1;
    m_commits = 0;
    return true;
}
void Slot::add_prepare() { m_prepares += 1; }
size_t Slot::prepares() const { return m_prepares; }
void Slot::add_commit() { m_commits += 1; }
size_t Slot::commits() const { return m_commits; }
Operation const *Slot::operation() const { return &m_op; }

size_t OpLog::pre_prepare_op(Operation op) {
    // TODO: fix
    if (paxos_sgx::pbft::persistence::log_accepted(
            m_log.size(), op.m_account, op.m_amount, op.m_to) == -1) {
        ERROR("failed to log accepted slot %zu", m_log.size());
    }
    m_log.emplace_back(op);
    return m_log.size() - 1;
}
bool OpLog::add_op(size_t slot_n, Operation op) {
    // TODO: fix
    if (paxos_sgx::pbft::persistence::log_accepted(
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

bool OpLog::check_op(size_t slot_n,
                     paxos_sgx::pbft::OperationArgs const *op) const {
    Operation const *local_op = get_operation(slot_n);
    return local_op->m_account == op->account() && local_op->m_to == op->to() &&
           local_op->m_amount == op->amount();
}

void OpLog::add_prepare(size_t slot_n) { m_log[slot_n].add_prepare(); }
void OpLog::add_commit(size_t slot_n) {
    m_log[slot_n].add_commit();
    if (is_committed(slot_n)) {
        m_commited = slot_n;
    }
}

void OpLog::executed(size_t slot_n) { m_executed = slot_n; }

bool OpLog::is_prepared(size_t slot_n) const {
    return slot_n < m_log.size() &&
           m_log[slot_n].prepares() >= paxos_sgx::pbft::setup::quorum_size();
}
bool OpLog::is_committed(size_t slot_n) const {
    return slot_n < m_log.size() &&
           m_log[slot_n].commits() >= paxos_sgx::pbft::setup::quorum_size();
}
bool OpLog::is_executed(size_t slot_n) const {
    return ((ssize_t)slot_n) <= m_executed;
}
bool OpLog::can_execute(size_t slot_n) const {
    return ((ssize_t)slot_n) > m_executed && is_committed(slot_n) &&
           (slot_n == 0 || is_executed(slot_n - 1));
}

ssize_t OpLog::execution_cursor() const { return m_executed; }
ssize_t OpLog::committed_cursor() const { return m_commited; }
Operation const *OpLog::get_operation(size_t slot_n) const {
    return m_log[slot_n].operation();
}

}  // namespace pbft
}  // namespace paxos_sgx
