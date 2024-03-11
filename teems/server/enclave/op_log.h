/**
 * op_log.h
 *
 * The log of operations to execute
 */

#pragma once
#include <sys/types.h>
#include <vector>
#include "policy.h"
#include "teems_generated.h"

namespace teems {

class Operation {
   public:
    Operation() = default;
    Operation(int64_t key, bool policy_read, ServerPolicy policy)
        : m_key(key),
          m_policy_read(policy_read),
          m_policy(policy),
          m_valid(true) {}

    inline bool valid() const { return m_valid; }

    int64_t m_key = -1;
    bool m_policy_read = false;
    ServerPolicy m_policy;
    bool m_valid = false;
};

class Slot {
   public:
    Slot() = default;
    Slot(Operation op, bool sus)
        : m_op(op), m_accepts(1), m_suspicions(sus ? 1 : 0) {}
    ~Slot() = default;

    bool replace_op(Operation op, bool sus);
    void add_accept(bool sus);
    size_t accepts() const;
    size_t suspicions() const;
    Operation const *operation() const;

   private:
    Operation m_op;
    size_t m_accepts = 0;
    size_t m_suspicions = 0;
};

class OpLog {
   public:
    OpLog() = default;
    ~OpLog() = default;

    // returns the slot number
    size_t propose_op(Operation op, bool sus);
    bool add_op(size_t slot_n, Operation op, bool sus);
    void add_accept(size_t slot_n, bool sus);

    void executed(size_t slot_n);
    ssize_t execution_cursor() const;
    ssize_t accepted_cursor() const;
    ssize_t last_seen() const;

    bool is_accepted(size_t slot_n) const;
    bool is_executed(size_t slot_n) const;
    bool can_execute(size_t slot_n) const;

    Operation const *get_operation(size_t slot_n) const;

   private:
    void add_empty_slot();
    void add_slot(Operation &&op, bool sus);

    std::vector<Slot> m_log;
    ssize_t m_executed = -1;
    ssize_t m_accepted = -1;
};

}  // namespace teems
