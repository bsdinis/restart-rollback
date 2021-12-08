/**
 * replicas.h
 *
 * helpers to handle replicas
 */

#include <string.h>
#include <cstdint>

namespace paxos_sgx {
namespace crash {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);

}  // namespace replicas
}  // namespace crash
}  // namespace paxos_sgx
