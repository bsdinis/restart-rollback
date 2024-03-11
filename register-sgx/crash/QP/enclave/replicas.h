/**
 * replicas.h
 *
 * helpers to handle replicas
 */
#pragma once

#include <string.h>
#include <cstdint>

namespace register_sgx {
namespace crash {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);

}  // namespace replicas
}  // namespace crash
}  // namespace register_sgx
