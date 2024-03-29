/**
 * replicas.h
 *
 * helpers to handle replicas
 */
#pragma once

#include <string.h>
#include <cstdint>

namespace paxos_sgx {
namespace crash {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);
void execute(size_t slot_number);
void add_accept(size_t slot_number);

}  // namespace replicas
}  // namespace crash
}  // namespace paxos_sgx
