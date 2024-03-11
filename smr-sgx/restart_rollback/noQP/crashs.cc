/**
 * basicQP.c
 */

#include "config.h"
#include "log.h"
#include "main_loop.h"
#include "perf.h"
#include "setup.h"

#include <cerrno>
#include <cstdio>
#include <string>

#include <execinfo.h>
#include <unistd.h>

#include <signal.h>
#include <stdarg.h>
#include <sys/signal.h>
#include <unistd.h>

using namespace paxos_sgx::restart_rollback;

ssize_t my_idx = -1;

namespace {
// perf
//
// XXX: CHANGE ME
// add other operations
perf::perf_stats stats{{"sum"}, {"ping", perf::measure_type::COUNT}};

// signal things
int setup_signals(void (*handler)(int), int n_sigs, ...);
void clean_exit_handler(int sig_number);
void ignore_handler(int sig_number);
void restart_rollback_handler(int sig_number);
void shutdown_properly(int code, void *__);

}  // anonymous namespace

// app entry poing
int main(int argc, char *argv[]) {
    setlinebuf(stdout);
    if (argc != 3) {
        ERROR("usage: %s <config_file> <idx>", argv[0]);
        return -1;
    }
    if (setup_signals(clean_exit_handler, 8, SIGINT, SIGTERM, SIGABRT, SIGALRM,
                      SIGHUP, SIGINT, SIGQUIT, SIGTSTP) == -1)
        KILL("failed to setup signals");
    if (setup_signals(ignore_handler, 2, SIGPIPE, SIGCHLD) == -1)
        KILL("failed to setup signals");
    if (setup_signals(restart_rollback_handler, 6, SIGSEGV, SIGFPE, SIGQUIT, SIGTSTP,
                      SIGURG, SIGXCPU) == -1)
        KILL("failed to setup signals");

    if (on_exit(shutdown_properly, NULL) == -1)
        KILL("failed to register shutdown function");

    my_idx = atoi(argv[2]);

    LOG("initial setup done");

    config_t conf;
    if (config_parse(&conf, argv[1]) != 0) KILL("failed to parse config");
    LOG("config setup");

    INFO("starting QP");
    basicQP_enclave_start(conf, my_idx, stats.data(), stats.size());
    config_free(&conf);
}

namespace {

// =================== SIGNALS ==========================

int setup_signals(void (*handler)(int), int n_sigs, ...) {
    va_list args;
    va_start(args, n_sigs);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    for (int i = 0; i < n_sigs; i++) {
        int signo = va_arg(args, int);
        if (sigaction(signo, &act, NULL) == -1) {
            ERROR("sigaction failed: %s", strerror(errno));
            va_end(args);
            return -1;
        }
    }

    va_end(args);
    return 0;
}

void clean_exit_handler(int sig_number) {
    LOG("caught signal %s; exiting", strsignal(sig_number));
    static sig_atomic_t volatile done_yet = 0;
    if (done_yet == 1) {
        ERROR("CALLED SHUTDOWN TWICE");
        raise(SIGKILL);
    } else {
        done_yet = 1;
        exit(EXIT_SUCCESS);
    }
}

void restart_rollback_handler(int sig_number) {
    LOG("caught signal %s; exiting", strsignal(sig_number));
    stats.print(stderr);
    stats.print(stdout);
    int nptrs;
    void *buffer[1024];

    nptrs = backtrace(buffer, 1024);
    LOG("[%ld] backtrace() returned %d addresses\n", my_idx, nptrs);
    backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO);
    fflush(stdout);
    raise(SIGKILL);
}

void ignore_handler(int sig_number) {
    FINE("caught signal %s; ignoring", strsignal(sig_number));
    fflush(stderr);
    fflush(stdout);
    errno = 0;
}

void shutdown_properly(int code, void *__) {
    (void)__;
    paxos_sgx::restart_rollback::setup::close();
    stats.print(stderr);
    stats.print(stdout);
    LOG("shutdown basicQP properly");
}

}  // anonymous namespace
