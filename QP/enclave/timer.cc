/**
 * timer.cc
 *
 * INTENSIVE_PERF sensitive timer
 */

#include "timer.h"

#ifndef INTRUSIVE_PERF

namespace epidemics {
namespace basicQP {
namespace timer {

struct timeval now() {
    return {0, 0};
}
double elapsed_usec(struct timeval const begin) { return 0.0; }
double elapsed_msec(struct timeval const begin) { return 0.0; }
}  // namespace timer
}  // namespace basicQP
}  // namespace epidemics
#else

#include "enclave_t.h"
#include "log.h"

namespace epidemics {
namespace basicQP {
namespace timer {

struct timeval now() {
    struct timeval n;
    int ret;
    sgx_status_t status =
        ocall_sgx_gettimeofday(&ret, &n, sizeof(struct timeval));
    if (status != SGX_SUCCESS) {
        ERROR("sgx: gettimeofday failed");
        return {0, 0};
    }

    if (ret == -1) {
        ERROR("gettimeofday failed");
        return {0, 0};
    }

    return n;
}

double elapsed_usec(struct timeval const begin) {
    auto const n = now();
    return static_cast<double>((n.tv_sec - begin.tv_sec) * 1000000 + n.tv_usec -
                               begin.tv_usec);
}
double elapsed_msec(struct timeval const begin) {
    return elapsed_usec(begin) / 1000;
}

}  // namespace timer
}  // namespace basicQP
}  // namespace epidemics

#endif
