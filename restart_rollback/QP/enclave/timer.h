/**
 * timer.h
 *
 * INTENSIVE_PERF sensitive timer
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
typedef long int __suseconds_t;

#ifndef __timeval_defined
#define __timeval_defined
#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL
struct timeval {
    __time_t tv_sec;
    __suseconds_t tv_usec;
};
#endif
#endif

namespace register_sgx {
namespace restart_rollback {
namespace timer {

struct timeval now();

double elapsed_usec(struct timeval const begin);
double elapsed_msec(struct timeval const begin);

}  // namespace timer
}  // namespace restart_rollback
}  // namespace register_sgx
