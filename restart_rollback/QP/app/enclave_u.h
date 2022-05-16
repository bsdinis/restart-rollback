#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "config.h"
#include "ocall_types.h"
#include "time.h"
#include "user_types.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_START_ENCLAVE_REPORT_DEFINED__
#define OCALL_START_ENCLAVE_REPORT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_start_enclave_report,
                 (const char* str));
#endif
#ifndef OCALL_SGX_OLOG_DEFINED__
#define OCALL_SGX_OLOG_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_olog, (const char* str));
#endif
#ifndef OCALL_SGX_ELOG_DEFINED__
#define OCALL_SGX_ELOG_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_elog, (const char* str));
#endif
#ifndef OCALL_SGX_EOLOG_DEFINED__
#define OCALL_SGX_EOLOG_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_eolog, (const char* str));
#endif
#ifndef OCALL_SGX_GETTIMEOFDAY_DEFINED__
#define OCALL_SGX_GETTIMEOFDAY_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_gettimeofday,
                (void* tv, int tv_size));
#endif
#ifndef OCALL_SGX_GETSOCKOPT_DEFINED__
#define OCALL_SGX_GETSOCKOPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_getsockopt,
                (int s, int level, int optname, char* optval, int optval_len,
                 int* optlen));
#endif
#ifndef OCALL_SGX_SETSOCKOPT_DEFINED__
#define OCALL_SGX_SETSOCKOPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_setsockopt,
                (int s, int level, int optname, const void* optval,
                 int optlen));
#endif
#ifndef OCALL_SGX_SOCKET_DEFINED__
#define OCALL_SGX_SOCKET_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_socket,
                (int af, int type, int protocol));
#endif
#ifndef OCALL_SGX_LISTEN_DEFINED__
#define OCALL_SGX_LISTEN_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_listen, (int s, int backlog));
#endif
#ifndef OCALL_SGX_BIND_DEFINED__
#define OCALL_SGX_BIND_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_bind,
                (int s, const void* addr, int addr_size));
#endif
#ifndef OCALL_SGX_CONNECT_DEFINED__
#define OCALL_SGX_CONNECT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_connect,
                (int s, const void* addr, int addrlen));
#endif
#ifndef OCALL_SGX_CONNECT_BLOCK_DEFINED__
#define OCALL_SGX_CONNECT_BLOCK_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_connect_block,
                (int* socket, const char* addr, uint16_t port));
#endif
#ifndef OCALL_SGX_ACCEPT_DEFINED__
#define OCALL_SGX_ACCEPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_accept,
                (int s, void* addr, int addr_size, int* addrlen));
#endif
#ifndef OCALL_SGX_ACCEPT_BLOCK_DEFINED__
#define OCALL_SGX_ACCEPT_BLOCK_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_accept_block, (int s));
#endif
#ifndef OCALL_SGX_SHUTDOWN_DEFINED__
#define OCALL_SGX_SHUTDOWN_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_shutdown, (int fd));
#endif
#ifndef OCALL_SGX_READ_DEFINED__
#define OCALL_SGX_READ_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_read, (int fd, void* buf, int n));
#endif
#ifndef OCALL_SGX_WRITE_DEFINED__
#define OCALL_SGX_WRITE_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_write,
                (int fd, const void* buf, int n));
#endif
#ifndef OCALL_SGX_CLOSE_DEFINED__
#define OCALL_SGX_CLOSE_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_close, (int fd));
#endif
#ifndef OCALL_SGX_GETENV_DEFINED__
#define OCALL_SGX_GETENV_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_getenv,
                (const char* env, int envlen, char* ret_str, int ret_len));
#endif
#ifndef OCALL_SGX_SELECT_DEFINED__
#define OCALL_SGX_SELECT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_select, (int high_sock));
#endif
#ifndef OCALL_SGX_EXIT_DEFINED__
#define OCALL_SGX_EXIT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_exit, (int code));
#endif
#ifndef OCALL_SGX_PUTS_DEFINED__
#define OCALL_SGX_PUTS_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_puts, (const char* str));
#endif
#ifndef OCALL_SGX_PUTS_E_DEFINED__
#define OCALL_SGX_PUTS_E_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_puts_e, (const char* str));
#endif
#ifndef OCALL_SGX_FLUSH_E_DEFINED__
#define OCALL_SGX_FLUSH_E_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_flush_e, (void));
#endif
#ifndef OCALL_SGX_CLOCK_DEFINED__
#define OCALL_SGX_CLOCK_DEFINED__
long int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_clock, (void));
#endif
#ifndef OCALL_SGX_TIME_DEFINED__
#define OCALL_SGX_TIME_DEFINED__
time_t SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_time,
                   (time_t * timep, int t_len));
#endif
#ifndef OCALL_SGX_LOCALTIME_DEFINED__
#define OCALL_SGX_LOCALTIME_DEFINED__
struct tm* SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_localtime,
                       (const time_t* timep, int t_len));
#endif
#ifndef OCALL_SGX_GMTIME_R_DEFINED__
#define OCALL_SGX_GMTIME_R_DEFINED__
struct tm* SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sgx_gmtime_r,
                       (const time_t* timep, int t_len, struct tm* tmp,
                        int tmp_len));
#endif
#ifndef OCALL_LOG_DEFINED__
#define OCALL_LOG_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_log,
                 (const char* file, int line, const char* str));
#endif
#ifndef OCALL_BUILD_ADDR_DEFINED__
#define OCALL_BUILD_ADDR_DEFINED__
void* SGX_UBRIDGE(SGX_NOCONVENTION, ocall_build_addr,
                  (uint16_t port, const char* ip_addr));
#endif
#ifndef OCALL_NET_START_LISTEN_SOCKET_DEFINED__
#define OCALL_NET_START_LISTEN_SOCKET_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_net_start_listen_socket,
                (const char* server_addr, uint16_t server_port));
#endif
#ifndef OCALL_NET_GET_MY_IPV4_ADDR_DEFINED__
#define OCALL_NET_GET_MY_IPV4_ADDR_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_net_get_my_ipv4_addr,
                 (char* addr, size_t sz));
#endif
#ifndef OCALL_NEEDS_READ_DEFINED__
#define OCALL_NEEDS_READ_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_needs_read, (int sock));
#endif
#ifndef OCALL_NEEDS_WRITE_DEFINED__
#define OCALL_NEEDS_WRITE_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_needs_write, (int sock));
#endif
#ifndef OCALL_NEEDS_EXCEPT_DEFINED__
#define OCALL_NEEDS_EXCEPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_needs_except, (int sock));
#endif
#ifndef OCALL_SELECT_LIST_DEFINED__
#define OCALL_SELECT_LIST_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_select_list,
                (selected_t * list, ssize_t list_sz));
#endif
#ifndef SGX_OC_CPUIDEX_DEFINED__
#define SGX_OC_CPUIDEX_DEFINED__
void SGX_UBRIDGE(SGX_CDECL, sgx_oc_cpuidex,
                 (int cpuinfo[4], int leaf, int subleaf));
#endif
#ifndef SGX_THREAD_WAIT_UNTRUSTED_EVENT_OCALL_DEFINED__
#define SGX_THREAD_WAIT_UNTRUSTED_EVENT_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_wait_untrusted_event_ocall,
                (const void* self));
#endif
#ifndef SGX_THREAD_SET_UNTRUSTED_EVENT_OCALL_DEFINED__
#define SGX_THREAD_SET_UNTRUSTED_EVENT_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_untrusted_event_ocall,
                (const void* waiter));
#endif
#ifndef SGX_THREAD_SETWAIT_UNTRUSTED_EVENTS_OCALL_DEFINED__
#define SGX_THREAD_SETWAIT_UNTRUSTED_EVENTS_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_setwait_untrusted_events_ocall,
                (const void* waiter, const void* self));
#endif
#ifndef SGX_THREAD_SET_MULTIPLE_UNTRUSTED_EVENTS_OCALL_DEFINED__
#define SGX_THREAD_SET_MULTIPLE_UNTRUSTED_EVENTS_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_multiple_untrusted_events_ocall,
                (const void** waiters, size_t total));
#endif

sgx_status_t restart_rollback_enclave_start(sgx_enclave_id_t eid, config_t conf,
                                            ssize_t my_idx, void* mmaped_addr,
                                            size_t addr_size,
                                            void* measure_buffer,
                                            size_t buffer_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
