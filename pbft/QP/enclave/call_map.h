/**
 * call_map.h
 *
 * The map of outstanding calls
 */

#pragma once
#include <stdint.h>
#include <unordered_map>

namespace paxos_sgx {
namespace pbft {

class CallMap {
   public:
    void add_call(size_t slot_n, int64_t ticket);
    void resolve_call(size_t slot_n, int64_t account, int64_t amount,
                      bool success);

   private:
    std::unordered_map<size_t, int64_t> m_map;
};

}  // namespace pbft
}  // namespace paxos_sgx
