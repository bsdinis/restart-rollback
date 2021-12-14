/**
 * replicas.h
 *
 * helpers to handle replicas
 */
#pragma once

#include <string.h>
#include <cstdint>
#include "pbft_generated.h"

namespace paxos_sgx {
namespace pbft {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);
void execute(size_t slot_number);
bool add_prepare(int64_t ticket, size_t slot_number,
                 paxos_sgx::pbft::OperationArgs const *op);
void add_commit(size_t slot_number, paxos_sgx::pbft::OperationArgs const *op);

}  // namespace replicas
}  // namespace pbft
}  // namespace paxos_sgx
