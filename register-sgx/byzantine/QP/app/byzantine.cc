/**
 * byzantine.c
 */

#include "byzantine.h"

#include "config.h"
#include "log.h"
#include "perf.h"

#include <cerrno>
#include <cstdio>
#include <string>

#include <execinfo.h>
#include <unistd.h>

#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace register_sgx::byzantine;

ssize_t my_idx = -1;
constexpr size_t MAPPED_SIZE = 1 << 20;

namespace {
// SGX enclave ID
sgx_enclave_id_t global_eid = 0;
int sgx_init_enclave(void);

// perf
//
// XXX: CHANGE ME
// add other operations
perf::perf_stats stats{{"sum"}, {"ping", perf::measure_type::COUNT}};

void setup_config(config_t const *conf, std::string const &filename);

// signal things
int setup_signals(void (*handler)(int), int n_sigs, ...);
void clean_exit_handler(int sig_number);
void ignore_handler(int sig_number);
void byzantine_handler(int sig_number);
void shutdown_properly(int code, void *__);
void *create_mapping(char const *filename);

}  // anonymous namespace

// app entry poing
int SGX_CDECL main(int argc, char *argv[]) {
    setlinebuf(stdout);
    if (argc != 4) {
        ERROR("usage: %s <config_file> <store_file> <idx>", argv[0]);
        return -1;
    }
    if (setup_signals(clean_exit_handler, 8, SIGINT, SIGTERM, SIGABRT, SIGALRM,
                      SIGHUP, SIGINT, SIGQUIT, SIGTSTP) == -1)
        KILL("failed to setup signals");
    if (setup_signals(ignore_handler, 2, SIGPIPE, SIGCHLD) == -1)
        KILL("failed to setup signals");
    if (setup_signals(byzantine_handler, 6, SIGSEGV, SIGFPE, SIGQUIT, SIGTSTP,
                      SIGURG, SIGXCPU) == -1)
        KILL("failed to setup signals");

    if (on_exit(shutdown_properly, NULL) == -1)
        KILL("failed to register shutdown function");

    my_idx = atoi(argv[3]);
    char mapping_filename[1024];
    mapping_filename[1023] = 0;
    snprintf(mapping_filename, 1023, "%s/%zd_%s", dirname(argv[2]), my_idx,
             basename(argv[2]));

    LOG("initial setup done");
    if (sgx_init_enclave() < 0) {
        return -1;
    }
    LOG("enclave initialized");

    config_t conf;
    if (config_parse(&conf, argv[1]) != 0) KILL("failed to parse config");
    LOG("config setup");

    INFO("starting enclave");
    byzantine_enclave_start(global_eid, conf, my_idx,
                            create_mapping(mapping_filename), MAPPED_SIZE,
                            stats.data(), stats.size(), (conf.size - 1) / 3);
    config_free(&conf);
}

namespace {

// print sgx errors
void sgx_print_error(sgx_status_t ret) {
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if (ret == sgx_errlist[idx].err) {
            if (NULL != sgx_errlist[idx].sug) INFO("%s", sgx_errlist[idx].sug);
            ERROR("%s", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        INFO(
            "Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer "
            "Reference\" for more details.",
            ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int sgx_init_enclave(void) {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL,
                             &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        sgx_print_error(ret);
        return -1;
    }
    return 0;
}

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

void byzantine_handler(int sig_number) {
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
    stats.print(stderr);
    stats.print(stdout);
    if (sgx_destroy_enclave(global_eid) != SGX_SUCCESS) {
        ERROR("failed to destroy enclave");
    }
    FINE("closed the enclave", my_idx, getpid());
    LOG("shutdown byzantine properly");
}

void *create_mapping(char const *filename) {
    int fd =
        open(filename, O_CREAT | O_TRUNC | O_RDWR | O_SYNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        KILL("failed to open file %s: %s", filename, strerror(errno));
    }

    if (ftruncate(fd, MAPPED_SIZE) == -1) {
        KILL("failed to truncate file %s: %s", filename, strerror(errno));
    }

    void *addr =
        mmap(NULL, MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        KILL("failed to mmap file %s: %s", filename, strerror(errno));
    }

    if (close(fd) == -1) {
        KILL("failed to close file %s: %s", filename, strerror(errno));
    }

    return addr;
}

}  // anonymous namespace
