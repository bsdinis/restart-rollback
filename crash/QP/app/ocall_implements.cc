#include "ocall_implements.h"
#include "log.h"
#include "network_wrappers.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <cassert>

using namespace paxos_sgx::crash;

extern ssize_t my_idx;

long ocall_sgx_clock(void) {
    struct timespec tstart = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    return tstart.tv_sec * 1000000 +
           tstart.tv_nsec / 1000;  // Return micro seconds
}

time_t ocall_sgx_time(time_t *timep, int t_len) { return time(timep); }

struct tm *ocall_sgx_localtime(const time_t *timep, int t_len) {
    return localtime(timep);
}

struct tm *ocall_sgx_gmtime_r(const time_t *timep, int t_len, struct tm *tmp,
                              int tmp_len) {
    return gmtime_r(timep, tmp);
}

int ocall_sgx_gettimeofday(void *tv, int tv_size) {
    return gettimeofday((struct timeval *)tv, NULL);
}

int ocall_sgx_getsockopt(int s, int level, int optname, char *optval,
                         int optval_len, int *optlen) {
    return getsockopt(s, level, optname, optval, (socklen_t *)optlen);
}

int ocall_sgx_setsockopt(int s, int level, int optname, const void *optval,
                         int optlen) {
    return setsockopt(s, level, optname, optval, optlen);
}

int ocall_sgx_socket(int af, int type, int protocol) {
    return socket(af, type, protocol);
}

int ocall_sgx_bind(int s, const void *addr, int addr_size) {
    return bind(s, (struct sockaddr *)addr, addr_size);
}

int ocall_sgx_listen(int s, int backlog) { return listen(s, backlog); }

int ocall_sgx_connect(int s, const void *addr, int addrlen) {
    int retv = connect(s, (struct sockaddr *)addr, addrlen);
    return retv;
}

int ocall_sgx_connect_block(int *s, const char *addr, uint16_t server_port) {
    int _socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (_socket < 0) {
        ERROR("failed to open socket");
        return -1;
    }

#ifdef DISABLE_NAGLE
    {
        int one = 1;
        if (setsockopt(_socket, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) ==
            -1) {
            KILL("setsockopt failed: %s", strerror(errno));
        }
    }
#endif

    *s = _socket;

    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, addr, &(s_addr.sin_addr)) <= 0) {
        ERROR("inet_pton");
        return -1;
    }

    errno = 0;
    while (connect(_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1 &&
           (errno == EINPROGRESS || errno == EALREADY))
        ;

    if (errno != 0 && errno != EINPROGRESS && errno != EALREADY) {
        ERROR("failed to connect to %s:%d - %s", addr, server_port,
              strerror(errno));
        return -1;
    }
    errno = 0;

    return 0;
}

int ocall_sgx_accept(int s, void *addr, int addr_size, int *addrlen) {
    int sock = accept(s, (struct sockaddr *)addr, (socklen_t *)addrlen);
    if (sock == -1) {
        ERROR("failed to accept on socket %d: %s", s, strerror(errno));
        return -1;
    }
#ifdef DISABLE_NAGLE
    {
        int one = 1;
        if (setsockopt(sock, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1) {
            KILL("setsockopt failed: %s", strerror(errno));
        }
    }
#endif
    return sock;
}

int ocall_sgx_accept_block(int listen_socket) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    int sock = accept(listen_socket, (struct sockaddr *)&addr, &len);

    if (sock == -1) {
        ERROR("failed to accept on socket %d: %s", listen_socket,
              strerror(errno));
        return -1;
    }
#ifdef DISABLE_NAGLE
    {
        int one = 1;
        if (setsockopt(sock, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1) {
            KILL("setsockopt failed: %s", strerror(errno));
        }
    }
#endif
    return sock;
}

int ocall_sgx_shutdown(int fd, int how) { return shutdown(fd, how); }

int ocall_sgx_read(int fd, void *buf, int n) {
    errno = 0;
    int ret = read(fd, buf, n);
    if (ret >= 0)
        return ret;
    else if (errno == EAGAIN || errno == EWOULDBLOCK)
        return 0;

    ERROR("%s failed (%d): %s", __func__, errno, strerror(errno));
    errno = 0;
    return -1;
}

int ocall_sgx_write(int fd, const void *buf, int n) {
    errno = 0;
    int ret = write(fd, buf, n);
    if (ret >= 0)
        return ret;
    else if (errno == EAGAIN || errno == EWOULDBLOCK)
        return 0;

    ERROR("%s failed (%d): %s", __func__, errno, strerror(errno));
    errno = 0;
    return -1;
}

int ocall_sgx_close(int fd) { return close(fd); }

int ocall_sgx_getenv(const char *env, int envlen, char *ret_str, int ret_len) {
    const char *env_val = getenv(env);
    if (env_val == NULL) {
        return -1;
    }
    memcpy(ret_str, env_val, strlen(env_val) + 1);
    return 0;
}

void ocall_sgx_puts(const char *str) {
    fputs(str, stdout);
    fflush(stdout);
}

void ocall_sgx_exit(int e) {
    INFO("called exit with %d", e);
    exit(e);
}

void ocall_sgx_flush_e(void) { fflush(stderr); }

int ocall_lstat(const char *path, struct stat *buf, int size) {
    // printf("Entering %s\n", __func__);
    return lstat(path, buf);
}

int ocall_stat(const char *path, struct stat *buf, int size) {
    // printf("Entering %s\n", __func__);
    return stat(path, buf);
}

int ocall_fstat(int fd, struct stat *buf, int size) {
    // printf("Entering %s\n", __func__);
    return fstat(fd, buf);
}

int ocall_ftruncate(int fd, off_t length) {
    // printf("Entering %s\n", __func__);
    return ftruncate(fd, length);
}

char *ocall_getcwd(char *buf, size_t size) {
    // printf("Entering %s\n", __func__);
    return getcwd(buf, size);
}

int ocall_getpid(void) {
    // printf("Entering %s\n", __func__);
    return getpid();
}

int ocall_open64(const char *filename, int flags, mode_t mode) {
    // printf("Entering %s\n", __func__);
    return open(filename, flags,
                mode);  // redirect it to open() instead of open64()
}

off_t ocall_lseek64(int fd, off_t offset, int whence) {
    // printf("Entering %s\n", __func__);
    return lseek(fd, offset,
                 whence);  // redirect it to lseek() instead of lseek64()
}

int ocall_read(int fd, void *buf, size_t count) {
    errno = 0;
    int ret = read(fd, buf, count);
    if (ret >= 0) return 0;

    if (ret == -1) {
        ERROR("read failed with %d: %s", errno, strerror(errno));
        ERROR("arguments (%d, %p, %zu)", fd, buf, count);
    }
    return ret;
}

int ocall_write(int fd, const void *buf, size_t count) {
    errno = 0;
    int ret = write(fd, buf, count);
    if (ret >= 0) return 0;
    if (ret == -1) {
        ERROR("write failed with %d: %s", errno, strerror(errno));
        ERROR("arguments (%d, %p, %zu)", fd, buf, count);
    }
    return ret;
}

int ocall_fcntl(int fd, int cmd, void *arg, size_t size) {
    // printf("Entering %s\n", __func__);
    return fcntl(fd, cmd, arg);
}

int ocall_close(int fd) {
    // printf("Entering %s\n", __func__);
    return close(fd);
}

int ocall_unlink(const char *pathname) {
    // printf("Entering %s\n", __func__);
    return unlink(pathname);
}

int ocall_getuid(void) {
    // printf("Entering %s\n", __func__);
    return getuid();
}

char *ocall_getenv(const char *name) {
    // printf("Entering %s\n", __func__);
    return getenv(name);
}

int ocall_fsync(int fd) {
    // printf("Entering %s\n", __func__);
    return fsync(fd);
}

int ocall_fchmod(int fd, mode_t mode) {
    // printf("Entering %s\n", __func__);
    return fchmod(fd, mode);
}

/**
 * select stuff
 */

static fd_set read_fds;
static fd_set write_fds;
static fd_set except_fds;

int ocall_needs_read(int sock) { return FD_ISSET(sock, &read_fds) ? 1 : 0; }
int ocall_needs_write(int sock) { return FD_ISSET(sock, &write_fds) ? 1 : 0; }
int ocall_needs_except(int sock) { return FD_ISSET(sock, &except_fds) ? 1 : 0; }

int ocall_sgx_select(int high_sock) {
    return select(high_sock, &read_fds, &write_fds, &except_fds, NULL);
}

int ocall_select_list(selected_t *list, ssize_t list_sz) {
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    int high_sock = 0;

    for (ssize_t i = 0; i < list_sz; i++) {
        FD_SET(list[i].sockfd, &read_fds);
        FD_SET(list[i].sockfd, &except_fds);

        if (list[i].wr) FD_SET(list[i].sockfd, &write_fds);

        if (list[i].sockfd > high_sock) high_sock = list[i].sockfd;
    }

    int activity =
        select(high_sock + 1, &read_fds, &write_fds, &except_fds, NULL);
    if (activity == -1) ERROR("select returned -1: %s", strerror(errno));

    return activity;
}

void *ocall_build_addr(uint16_t port, const char *ip_addr) {
    struct sockaddr_in *addr =
        (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
    assert(addr != nullptr);

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (inet_pton(AF_INET, ip_addr, &(addr->sin_addr)) <= 0) {
        ERROR("inet_pton");
        free(addr);
        return NULL;
    }

    return (void *)addr;
}

int ocall_net_start_listen_socket(const char *server_addr,
                                  uint16_t server_port) {
    return __utils::net_start_listen_socket(server_addr, server_port);
}

void ocall_net_get_my_ipv4_addr(char *addr, size_t sz) {
    char const *__addr = __utils::net_get_my_ipv4_addr();
    if (__addr == NULL) {
        ERROR("failed to get my ip address");
        exit(EXIT_FAILURE);
    }

    strncpy(addr, __addr, sz - 1);
    addr[sz - 1] = 0;
}

void ocall_log(char const *filename, int line, char const *str) {
    fprintf(stderr, "[%ld] Enclave Report %s:%d: %s\n", my_idx, filename, line,
            str);
}

void ocall_start_enclave_report(const char *str) {
    fprintf(stderr, "[%ld] Enclave Report %s", my_idx, str);
}

void ocall_sgx_eolog(const char *str) {
    fprintf(stderr, "[SGX] %ld | %d | %s", my_idx, getpid(), str);
    fprintf(stdout, "[SGX] %ld | %d | %s", my_idx, getpid(), str);
}

void ocall_sgx_olog(const char *str) {
    fprintf(stdout, "[SGX] %ld | %d | %s", my_idx, getpid(), str);
}
void ocall_sgx_elog(const char *str) {
    fprintf(stderr, "[SGX] %ld | %d | %s", my_idx, getpid(), str);
}

void ocall_sgx_puts_e(const char *str) { fputs(str, stderr); }
