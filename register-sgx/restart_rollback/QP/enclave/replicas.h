/**
 * replicas.h
 *
 * helpers to handle replicas
 */
#pragma once

#include <string.h>
#include <cstdint>

namespace register_sgx {
namespace restart_rollback {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);

}  // namespace replicas
}  // namespace restart_rollback
}  // namespace register_sgx
