/**
 * timer.h
 *
 * INTENSIVE_PERF sensitive timer
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>

namespace paxos_sgx {
namespace restart_rollback {
namespace timer {

struct timeval now();

double elapsed_usec(struct timeval const begin);
double elapsed_msec(struct timeval const begin);

}  // namespace timer
}  // namespace restart_rollback
}  // namespace paxos_sgx
