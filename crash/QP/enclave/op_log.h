/**
 * op_log.h
 *
 * The log of operations to execute
 */

#pragma once
#include <sys/types.h>
#include <vector>
#include "crash_generated.h"

namespace paxos_sgx {
namespace crash {

class Operation {
   public:
    Operation() = default;
    Operation(paxos_sgx::crash::OperationArgs const *op);

    bool valid() const;

    int64_t m_account = -1;
    int64_t m_to = -1;
    int64_t m_amount = -1;
};

class Slot {
   public:
    Slot() = default;
    Slot(Operation op) : m_op(op), m_accepts(1) {}
    ~Slot() = default;

    Operation const *operation() const;
    size_t accepts() const;
    void add_accept();
    bool replace_op(Operation op);

   private:
    Operation m_op;
    size_t m_accepts = 0;
};

class OpLog {
   public:
    OpLog() = default;
    ~OpLog() = default;

    // returns the slot number
    size_t propose_op(Operation op);
    bool add_op(size_t slot_n, Operation op);
    void add_accept(size_t slot_n);

    void executed(size_t slot_n);
    ssize_t execution_cursor() const;
    ssize_t accepted_cursor() const;

    bool is_accepted(size_t slot_n) const;
    bool is_executed(size_t slot_n) const;
    bool can_execute(size_t slot_n) const;

    Operation const *get_operation(size_t slot_n) const;

   private:
    void add_empty_slot();
    void add_slot(Operation &&op);

    std::vector<Slot> m_log;
    ssize_t m_executed = -1;
    ssize_t m_accepted = -1;
};

}  // namespace crash
}  // namespace paxos_sgx
