/**
 * timer.cc
 *
 * INTENSIVE_PERF sensitive timer
 */

#include "timer.h"

#ifndef INTRUSIVE_PERF

namespace paxos_sgx {
namespace pbft {
namespace timer {

struct timeval now() {
    return {0, 0};
}
double elapsed_usec(struct timeval const begin) { return 0.0; }
double elapsed_msec(struct timeval const begin) { return 0.0; }
}  // namespace timer
}  // namespace pbft
}  // namespace paxos_sgx
#else

#include <sys/time.h>
#include <cerrno>
#include <cstring>
#include "log.h"

namespace paxos_sgx {
namespace pbft {
namespace timer {

struct timeval now() {
    struct timeval n;
    int ret = gettimeofday(&n, NULL);

    if (ret == -1) {
        ERROR("gettimeofday failed: %s", strerror(errno));
        return {0, 0};
    }

    return n;
}

double elapsed_usec(struct timeval const begin) {
    auto const n = now();
    return static_cast<double>((n.tv_sec - begin.tv_sec) * 1'000'000 +
                               n.tv_usec - begin.tv_usec);
}
double elapsed_msec(struct timeval const begin) {
    return elapsed_usec(begin) / 1'000;
}

}  // namespace timer
}  // namespace pbft
}  // namespace paxos_sgx

#endif
