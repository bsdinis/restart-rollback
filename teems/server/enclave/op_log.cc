/*
 * op_log.cc
 *
 * The log of operations to execute
 */

#include "op_log.h"
#include "log.h"
#include "persistence.h"
#include "setup.h"

namespace teems {

// =================================================
// Slot
// =================================================
Operation const *Slot::operation() const { return &m_op; }

size_t Slot::accepts() const { return m_accepts; }

size_t Slot::suspicions() const { return m_suspicions; }

void Slot::add_accept(bool sus) {
    m_accepts += 1;
    if (sus) {
        m_suspicions += 1;
    }
}
bool Slot::replace_op(Operation op, bool sus) {
    if (m_op.valid()) {
        return false;
    }
    m_op = op;
    m_accepts += 1;
    if (sus) {
        m_suspicions += 1;
    }
    return true;
}

// =================================================
// OpLog
// =================================================
void OpLog::add_empty_slot() {
    try {
        m_log.emplace_back();
    } catch (std::bad_alloc const &) {
        KILL("OOM: operation log: empty slot");
    }
}

void OpLog::add_slot(Operation &&op, bool sus) {
    try {
        m_log.emplace_back(op, sus);
    } catch (std::bad_alloc const &) {
        KILL("OOM: operation log");
    }
}

size_t OpLog::propose_op(Operation op, bool sus) {
    if (log_accepted(m_log.size(), op.m_key, op.m_policy_read, op.m_policy) ==
        -1) {
        ERROR("failed to log accepted slot %zu", m_log.size());
    }
    this->add_slot(std::move(op), sus);
    return m_log.size() - 1;
}

bool OpLog::add_op(size_t slot_n, Operation op, bool sus) {
    if (log_accepted(slot_n, op.m_key, op.m_policy_read, op.m_policy) == -1) {
        ERROR("failed to log accepted slot %zu", slot_n);
    }
    if (slot_n < m_log.size()) {
        return m_log[slot_n].replace_op(op, sus);
    }
    while (slot_n > m_log.size()) {
        this->add_empty_slot();
    }
    this->add_slot(std::move(op), sus);

    return true;
}
void OpLog::add_accept(size_t slot_n, bool sus) {
    while (slot_n >= m_log.size()) {
        this->add_empty_slot();
    }
    m_log[slot_n].add_accept(sus);
    if (slot_n > m_accepted && is_accepted(slot_n)) {
        m_accepted = slot_n;
    }
}

void OpLog::executed(size_t slot_n) { m_executed = slot_n; }

bool OpLog::is_accepted(size_t slot_n) const {
    return slot_n < m_log.size() &&
           m_log[slot_n].accepts() >=
               setup::max_quorum_size(m_log[slot_n].suspicions());
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

ssize_t OpLog::last_seen() const { return m_log.size() - 1; }

Operation const *OpLog::get_operation(size_t slot_n) const {
    return m_log[slot_n].operation();
}

}  // namespace teems
