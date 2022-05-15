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
void execute(size_t slot_number);
void add_accept(size_t slot_number, bool sus);

}  // namespace replicas
}  // namespace teems
