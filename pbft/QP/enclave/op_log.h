/**
 * op_log.h
 *
 * The log of operations to execute
 */

#pragma once
#include <sys/types.h>
#include <vector>
#include "pbft_generated.h"

namespace paxos_sgx {
namespace pbft {

class Operation {
   public:
    Operation() = default;
    Operation(paxos_sgx::pbft::OperationArgs const *op);

    bool valid() const;

    int64_t m_account = -1;
    int64_t m_to = -1;
    int64_t m_amount = -1;
};

class Slot {
   public:
    Slot() = default;
    Slot(Operation op) : m_op(op), m_prepares(1) {}
    ~Slot() = default;

    bool replace_op(Operation op);
    void add_prepare();
    void add_commit();

    size_t prepares() const;
    size_t commits() const;

    Operation const *operation() const;

   private:
    Operation m_op;
    size_t m_prepares = 0;
    size_t m_commits = 0;
};

class OpLog {
   public:
    OpLog() = default;
    ~OpLog() = default;

    // returns the slot number
    size_t pre_prepare_op(Operation op);
    bool add_op(size_t slot_n, Operation op);

    bool check_op(size_t slot_n,
                  paxos_sgx::pbft::OperationArgs const *op) const;

    void add_prepare(size_t slot_n);
    void add_commit(size_t slot_n);

    void executed(size_t slot_n);
    ssize_t execution_cursor() const;
    ssize_t committed_cursor() const;

    bool is_prepared(size_t slot_n) const;
    bool is_committed(size_t slot_n) const;
    bool is_executed(size_t slot_n) const;
    bool can_execute(size_t slot_n) const;

    Operation const *get_operation(size_t slot_n) const;

   private:
    void add_empty_slot();
    void add_slot(Operation &&op);

    std::vector<Slot> m_log;
    ssize_t m_executed = -1;
    ssize_t m_committed = -1;
};

}  // namespace pbft
}  // namespace paxos_sgx
