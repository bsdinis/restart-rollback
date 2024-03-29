#ifndef HELPERS_H
#define HELPERS_H

/**
 * IMPORTANT: include only in the context of OpenSSL
 */

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "enclave_t.h"
#include "sgx_trts.h"
#include "types.h"

#define assert ;

#define printf sgx_printf
#define printe(fmt, ...) sgx_printe(__FUNCTION__, fmt, ##__VA_ARGS__)
#define printl(fmt, ...) sgx_printl(__FUNCTION__, fmt, ##__VA_ARGS__)
#define printl_nodebug(fmt, ...) \
    sgx_printl_nodebug(__FUNCTION__, fmt, ##__VA_ARGS__)
#define exit sgx_exit

static char const hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

long sgx_clock(void); /* For Performance evaluation */
time_t sgx_time(time_t *timep);
struct tm *sgx_localtime(const time_t *timep);
struct tm *sgx_gmtime_r(const time_t *timep, struct tm *tmp);
int sgx_gettimeofday(struct timeval *tv);
int sgx_getsockopt(int s, int level, int optname, char *optval, int *optlen);
int sgx_setsockopt(int s, int level, int optname, const void *optval,
                   int optlen);
int sgx_socket(int af, int type, int protocol);
int sgx_bind(int s, const struct sockaddr *addr, int addrlen);
int sgx_listen(int s, int backlog);
int sgx_connect(int s, const struct sockaddr *addr, int addrlen);
int sgx_accept(int s, struct sockaddr *addr, int *addrlen);
int sgx_shutdown(int fd);
int sgx_read(int fd, void *buf, int n);
int sgx_write(int fd, const void *buf, int n);
int sgx_close(int fd);
char *sgx_getenv(const char *env);
void sgx_printf(const char *fmt, ...);
void sgx_printe(const char *fmt, ...);
void sgx_printe_start(const char *fmt, ...);
void sgx_printl(const char *fname, const char *fmt, ...);
void sgx_printl_nodebug(const char *fname, const char *fmt, ...);
long sgx_rand(void);
void sgx_exit(int exit_status);
int sgx_sscanf(const char *str, const char *format, ...);
int sgxssl_read_rand(unsigned char *rand_buf, int length_in_bytes);
const unsigned short **__ctype_b_loc(void);
const int32_t **__ctype_tolower_loc(void);

#endif
