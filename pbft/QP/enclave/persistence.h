/**
 * persitence.h
 *
 * handle persistence
 */

#pragma once

#include <sys/types.h>

namespace paxos_sgx {
namespace pbft {
namespace persistence {

int log_accepted(size_t slot_n, int64_t account, int64_t amount, int64_t to);

}  // namespace persistence
}  // namespace pbft
}  // namespace paxos_sgx
