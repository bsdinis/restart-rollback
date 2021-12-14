#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "user_types.h"
#include "config.h"
#include "time.h"
#include "ocall_types.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void restart_rollback_enclave_start(config_t conf, ssize_t my_idx, void* mmaped_addr, size_t addr_size, void* measure_buffer, size_t buffer_size);

sgx_status_t SGX_CDECL ocall_start_enclave_report(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_olog(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_elog(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_eolog(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_gettimeofday(int* retval, void* tv, int tv_size);
sgx_status_t SGX_CDECL ocall_sgx_getsockopt(int* retval, int s, int level, int optname, char* optval, int optval_len, int* optlen);
sgx_status_t SGX_CDECL ocall_sgx_setsockopt(int* retval, int s, int level, int optname, const void* optval, int optlen);
sgx_status_t SGX_CDECL ocall_sgx_socket(int* retval, int af, int type, int protocol);
sgx_status_t SGX_CDECL ocall_sgx_listen(int* retval, int s, int backlog);
sgx_status_t SGX_CDECL ocall_sgx_bind(int* retval, int s, const void* addr, int addr_size);
sgx_status_t SGX_CDECL ocall_sgx_connect(int* retval, int s, const void* addr, int addrlen);
sgx_status_t SGX_CDECL ocall_sgx_connect_block(int* retval, int* socket, const char* addr, uint16_t port);
sgx_status_t SGX_CDECL ocall_sgx_accept(int* retval, int s, void* addr, int addr_size, int* addrlen);
sgx_status_t SGX_CDECL ocall_sgx_accept_block(int* retval, int s);
sgx_status_t SGX_CDECL ocall_sgx_shutdown(int* retval, int fd);
sgx_status_t SGX_CDECL ocall_sgx_read(int* retval, int fd, void* buf, int n);
sgx_status_t SGX_CDECL ocall_sgx_write(int* retval, int fd, const void* buf, int n);
sgx_status_t SGX_CDECL ocall_sgx_close(int* retval, int fd);
sgx_status_t SGX_CDECL ocall_sgx_getenv(int* retval, const char* env, int envlen, char* ret_str, int ret_len);
sgx_status_t SGX_CDECL ocall_sgx_select(int* retval, int high_sock);
sgx_status_t SGX_CDECL ocall_sgx_exit(int code);
sgx_status_t SGX_CDECL ocall_sgx_puts(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_puts_e(const char* str);
sgx_status_t SGX_CDECL ocall_sgx_flush_e(void);
sgx_status_t SGX_CDECL ocall_sgx_clock(long int* retval);
sgx_status_t SGX_CDECL ocall_sgx_time(time_t* retval, time_t* timep, int t_len);
sgx_status_t SGX_CDECL ocall_sgx_localtime(struct tm** retval, const time_t* timep, int t_len);
sgx_status_t SGX_CDECL ocall_sgx_gmtime_r(struct tm** retval, const time_t* timep, int t_len, struct tm* tmp, int tmp_len);
sgx_status_t SGX_CDECL ocall_log(const char* file, int line, const char* str);
sgx_status_t SGX_CDECL ocall_build_addr(void** retval, uint16_t port, const char* ip_addr);
sgx_status_t SGX_CDECL ocall_net_start_listen_socket(int* retval, const char* server_addr, uint16_t server_port);
sgx_status_t SGX_CDECL ocall_net_get_my_ipv4_addr(char* addr, size_t sz);
sgx_status_t SGX_CDECL ocall_needs_read(int* retval, int sock);
sgx_status_t SGX_CDECL ocall_needs_write(int* retval, int sock);
sgx_status_t SGX_CDECL ocall_needs_except(int* retval, int sock);
sgx_status_t SGX_CDECL ocall_select_list(int* retval, selected_t* list, ssize_t list_sz);
sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf);
sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self);
sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter);
sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self);
sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
