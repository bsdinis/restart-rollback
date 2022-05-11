/**
 * replicas.h
 *
 * helpers to handle replicas
 */
#pragma once

#include <string.h>
#include <cstdint>

namespace teems {
namespace replicas {

int broadcast_message(uint8_t *message, size_t size);

}  // namespace replicas
}  // namespace teems
