#ifndef _OCALL_IMPLEMENTS_H_
#define _OCALL_IMPLEMENTS_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "enclave_u.h"

#include <sys/time.h>
#include <time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#define DO_NOT_REDEFINE_FOR_OCALL
#if defined(__cplusplus)
extern "C" {
#endif

long ocall_sgx_clock(void); /* For Performance evaluation */
time_t ocall_sgx_time(time_t *timep, int t_len);
struct tm *ocall_sgx_localtime(const time_t *timep, int t_len);
struct tm *ocall_sgx_gmtime_r(const time_t *timep, int t_len, struct tm *tmp,
                              int tmp_len);
int ocall_sgx_gettimeofday(void *tv, int tv_size);
int ocall_sgx_getsockopt(int s, int level, int optname, char *optval,
                         int optval_len, int *optlen);
int ocall_sgx_setsockopt(int s, int level, int optname, const void *optval,
                         int optlen);
int ocall_sgx_socket(int af, int type, int protocol);
int ocall_sgx_bind(int s, const void *addr, int addr_size);
int ocall_sgx_connect(int s, const void *addr, int addrlen);
int ocall_sgx_connect_block(int *s, const char *addr, uint16_t server_port);
int ocall_sgx_accept(int s, void *addr, int addr_size, int *addrlen);
int ocall_sgx_accept_block(int s);
int ocall_sgx_shutdown(int fd);
int ocall_sgx_read(int fd, void *buf, int n);
int ocall_sgx_write(int fd, const void *buf, int n);
int ocall_sgx_close(int fd);
int ocall_sgx_getenv(const char *env, int envlen, char *ret_str, int ret_len);

void ocall_print_string(const char *str);

int ocall_lstat(const char *path, struct stat *buf, int size);
int ocall_stat(const char *path, struct stat *buf, int size);
int ocall_fstat(int fd, struct stat *buf, int size);
int ocall_ftruncate(int fd, off_t length);
char *ocall_getcwd(char *buf, size_t size);
int ocall_getpid(void);
int ocall_open64(const char *filename, int flags, mode_t mode);
off_t ocall_lseek64(int fd, off_t offset, int whence);
int ocall_read(int fd, void *buf, size_t count);
int ocall_write(int fd, const void *buf, size_t count);
int ocall_fcntl(int fd, int cmd, void *arg, size_t size);
int ocall_close(int fd);
int ocall_unlink(const char *pathname);
int ocall_getuid(void);
char *ocall_getenv(const char *name);
int ocall_fsync(int fd);
int ocall_fchmod(int fd, mode_t mode);

int ocall_needs_read(int sock);
int ocall_needs_write(int sock);
int ocall_needs_except(int sock);
int ocall_sgx_select(int high_sock);
int ocall_select_list(selected_t *list, ssize_t list_sz);
void *ocall_build_addr(uint16_t port, const char *ip_addr);
int ocall_net_start_listen_socket(const char *server_addr,
                                  uint16_t server_port);
void ocall_net_get_my_ipv4_addr(char *addr, size_t sz);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
}
#endif
#endif /* !_OCALL_IMPLEMENTS_H_ */
