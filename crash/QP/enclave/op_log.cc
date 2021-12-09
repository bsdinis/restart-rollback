/*
 * op_log.cc
 *
 * The log of operations to execute
 */

#include "op_log.h"
#include "log.h"

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
    m_log.emplace_back(op);
    return m_log.size() - 1;
}
bool OpLog::add_op(size_t slot_n, Operation op) {
    if (slot_n < m_log.size()) {
        return m_log[slot_n].replace_op(op);
    }
    while (slot_n > m_log.size()) {
        m_log.emplace_back();
    }
    m_log.emplace_back(op);

    return true;
}
void OpLog::add_accept(size_t slot_n) { m_log[slot_n].add_accept(); }

void OpLog::executed(size_t slot_n) { m_executed = slot_n; }

size_t OpLog::get_accepts(size_t slot_n) const {
    return m_log[slot_n].accepts();
}
ssize_t OpLog::execution_cursor() const { return m_executed; }
Operation const *OpLog::get_operation(size_t slot_n) const {
    return m_log[slot_n].operation();
}

}  // namespace crash
}  // namespace paxos_sgx
