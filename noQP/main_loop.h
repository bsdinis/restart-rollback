/**
 * main_loop.h
 *
 * implement main interaction loop
 */

#pragma once

namespace epidemics {
namespace basicQP {

void basicQP_enclave_start(config_t conf, ssize_t my_idx, void *measure_buffer, size_t buffer_size);

}  // namespace basicQP
}  // namespace epidemics
