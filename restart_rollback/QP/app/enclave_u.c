#include "enclave_u.h"
#include <errno.h>

typedef struct ms_restart_rollback_enclave_start_t {
	config_t ms_conf;
	ssize_t ms_my_idx;
	void* ms_mmaped_addr;
	size_t ms_addr_size;
	void* ms_measure_buffer;
	size_t ms_buffer_size;
} ms_restart_rollback_enclave_start_t;

typedef struct ms_ocall_start_enclave_report_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_start_enclave_report_t;

typedef struct ms_ocall_sgx_olog_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_sgx_olog_t;

typedef struct ms_ocall_sgx_elog_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_sgx_elog_t;

typedef struct ms_ocall_sgx_eolog_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_sgx_eolog_t;

typedef struct ms_ocall_sgx_gettimeofday_t {
	int ms_retval;
	int ocall_errno;
	void* ms_tv;
	int ms_tv_size;
} ms_ocall_sgx_gettimeofday_t;

typedef struct ms_ocall_sgx_getsockopt_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	int ms_level;
	int ms_optname;
	char* ms_optval;
	int ms_optval_len;
	int* ms_optlen;
} ms_ocall_sgx_getsockopt_t;

typedef struct ms_ocall_sgx_setsockopt_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	int ms_level;
	int ms_optname;
	const void* ms_optval;
	int ms_optlen;
} ms_ocall_sgx_setsockopt_t;

typedef struct ms_ocall_sgx_socket_t {
	int ms_retval;
	int ocall_errno;
	int ms_af;
	int ms_type;
	int ms_protocol;
} ms_ocall_sgx_socket_t;

typedef struct ms_ocall_sgx_listen_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	int ms_backlog;
} ms_ocall_sgx_listen_t;

typedef struct ms_ocall_sgx_bind_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	const void* ms_addr;
	int ms_addr_size;
} ms_ocall_sgx_bind_t;

typedef struct ms_ocall_sgx_connect_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	const void* ms_addr;
	int ms_addrlen;
} ms_ocall_sgx_connect_t;

typedef struct ms_ocall_sgx_connect_block_t {
	int ms_retval;
	int ocall_errno;
	int* ms_socket;
	const char* ms_addr;
	uint16_t ms_port;
} ms_ocall_sgx_connect_block_t;

typedef struct ms_ocall_sgx_accept_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
	void* ms_addr;
	int ms_addr_size;
	int* ms_addrlen;
} ms_ocall_sgx_accept_t;

typedef struct ms_ocall_sgx_accept_block_t {
	int ms_retval;
	int ocall_errno;
	int ms_s;
} ms_ocall_sgx_accept_block_t;

typedef struct ms_ocall_sgx_shutdown_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
} ms_ocall_sgx_shutdown_t;

typedef struct ms_ocall_sgx_read_t {
	int ms_retval;
	int ms_fd;
	void* ms_buf;
	int ms_n;
} ms_ocall_sgx_read_t;

typedef struct ms_ocall_sgx_write_t {
	int ms_retval;
	int ms_fd;
	const void* ms_buf;
	int ms_n;
} ms_ocall_sgx_write_t;

typedef struct ms_ocall_sgx_close_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
} ms_ocall_sgx_close_t;

typedef struct ms_ocall_sgx_getenv_t {
	int ms_retval;
	int ocall_errno;
	const char* ms_env;
	int ms_envlen;
	char* ms_ret_str;
	int ms_ret_len;
} ms_ocall_sgx_getenv_t;

typedef struct ms_ocall_sgx_select_t {
	int ms_retval;
	int ocall_errno;
	int ms_high_sock;
} ms_ocall_sgx_select_t;

typedef struct ms_ocall_sgx_exit_t {
	int ocall_errno;
	int ms_code;
} ms_ocall_sgx_exit_t;

typedef struct ms_ocall_sgx_puts_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_sgx_puts_t;

typedef struct ms_ocall_sgx_puts_e_t {
	int ocall_errno;
	const char* ms_str;
} ms_ocall_sgx_puts_e_t;

typedef struct ms_ocall_sgx_flush_e_t {
	int ocall_errno;
} ms_ocall_sgx_flush_e_t;

typedef struct ms_ocall_sgx_clock_t {
	long int ms_retval;
	int ocall_errno;
} ms_ocall_sgx_clock_t;

typedef struct ms_ocall_sgx_time_t {
	time_t ms_retval;
	int ocall_errno;
	time_t* ms_timep;
	int ms_t_len;
} ms_ocall_sgx_time_t;

typedef struct ms_ocall_sgx_localtime_t {
	struct tm* ms_retval;
	int ocall_errno;
	const time_t* ms_timep;
	int ms_t_len;
} ms_ocall_sgx_localtime_t;

typedef struct ms_ocall_sgx_gmtime_r_t {
	struct tm* ms_retval;
	int ocall_errno;
	const time_t* ms_timep;
	int ms_t_len;
	struct tm* ms_tmp;
	int ms_tmp_len;
} ms_ocall_sgx_gmtime_r_t;

typedef struct ms_ocall_log_t {
	int ocall_errno;
	const char* ms_file;
	int ms_line;
	const char* ms_str;
} ms_ocall_log_t;

typedef struct ms_ocall_build_addr_t {
	void* ms_retval;
	int ocall_errno;
	uint16_t ms_port;
	const char* ms_ip_addr;
} ms_ocall_build_addr_t;

typedef struct ms_ocall_net_start_listen_socket_t {
	int ms_retval;
	int ocall_errno;
	const char* ms_server_addr;
	uint16_t ms_server_port;
} ms_ocall_net_start_listen_socket_t;

typedef struct ms_ocall_net_get_my_ipv4_addr_t {
	int ocall_errno;
	char* ms_addr;
	size_t ms_sz;
} ms_ocall_net_get_my_ipv4_addr_t;

typedef struct ms_ocall_needs_read_t {
	int ms_retval;
	int ocall_errno;
	int ms_sock;
} ms_ocall_needs_read_t;

typedef struct ms_ocall_needs_write_t {
	int ms_retval;
	int ocall_errno;
	int ms_sock;
} ms_ocall_needs_write_t;

typedef struct ms_ocall_needs_except_t {
	int ms_retval;
	int ocall_errno;
	int ms_sock;
} ms_ocall_needs_except_t;

typedef struct ms_ocall_select_list_t {
	int ms_retval;
	int ocall_errno;
	selected_t* ms_list;
	ssize_t ms_list_sz;
} ms_ocall_select_list_t;

typedef struct ms_sgx_oc_cpuidex_t {
	int* ms_cpuinfo;
	int ms_leaf;
	int ms_subleaf;
} ms_sgx_oc_cpuidex_t;

typedef struct ms_sgx_thread_wait_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_self;
} ms_sgx_thread_wait_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_set_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_waiter;
} ms_sgx_thread_set_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_setwait_untrusted_events_ocall_t {
	int ms_retval;
	const void* ms_waiter;
	const void* ms_self;
} ms_sgx_thread_setwait_untrusted_events_ocall_t;

typedef struct ms_sgx_thread_set_multiple_untrusted_events_ocall_t {
	int ms_retval;
	const void** ms_waiters;
	size_t ms_total;
} ms_sgx_thread_set_multiple_untrusted_events_ocall_t;

static sgx_status_t SGX_CDECL enclave_ocall_start_enclave_report(void* pms)
{
	ms_ocall_start_enclave_report_t* ms = SGX_CAST(ms_ocall_start_enclave_report_t*, pms);
	ocall_start_enclave_report(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_olog(void* pms)
{
	ms_ocall_sgx_olog_t* ms = SGX_CAST(ms_ocall_sgx_olog_t*, pms);
	ocall_sgx_olog(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_elog(void* pms)
{
	ms_ocall_sgx_elog_t* ms = SGX_CAST(ms_ocall_sgx_elog_t*, pms);
	ocall_sgx_elog(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_eolog(void* pms)
{
	ms_ocall_sgx_eolog_t* ms = SGX_CAST(ms_ocall_sgx_eolog_t*, pms);
	ocall_sgx_eolog(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_gettimeofday(void* pms)
{
	ms_ocall_sgx_gettimeofday_t* ms = SGX_CAST(ms_ocall_sgx_gettimeofday_t*, pms);
	ms->ms_retval = ocall_sgx_gettimeofday(ms->ms_tv, ms->ms_tv_size);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_getsockopt(void* pms)
{
	ms_ocall_sgx_getsockopt_t* ms = SGX_CAST(ms_ocall_sgx_getsockopt_t*, pms);
	ms->ms_retval = ocall_sgx_getsockopt(ms->ms_s, ms->ms_level, ms->ms_optname, ms->ms_optval, ms->ms_optval_len, ms->ms_optlen);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_setsockopt(void* pms)
{
	ms_ocall_sgx_setsockopt_t* ms = SGX_CAST(ms_ocall_sgx_setsockopt_t*, pms);
	ms->ms_retval = ocall_sgx_setsockopt(ms->ms_s, ms->ms_level, ms->ms_optname, ms->ms_optval, ms->ms_optlen);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_socket(void* pms)
{
	ms_ocall_sgx_socket_t* ms = SGX_CAST(ms_ocall_sgx_socket_t*, pms);
	ms->ms_retval = ocall_sgx_socket(ms->ms_af, ms->ms_type, ms->ms_protocol);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_listen(void* pms)
{
	ms_ocall_sgx_listen_t* ms = SGX_CAST(ms_ocall_sgx_listen_t*, pms);
	ms->ms_retval = ocall_sgx_listen(ms->ms_s, ms->ms_backlog);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_bind(void* pms)
{
	ms_ocall_sgx_bind_t* ms = SGX_CAST(ms_ocall_sgx_bind_t*, pms);
	ms->ms_retval = ocall_sgx_bind(ms->ms_s, ms->ms_addr, ms->ms_addr_size);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_connect(void* pms)
{
	ms_ocall_sgx_connect_t* ms = SGX_CAST(ms_ocall_sgx_connect_t*, pms);
	ms->ms_retval = ocall_sgx_connect(ms->ms_s, ms->ms_addr, ms->ms_addrlen);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_connect_block(void* pms)
{
	ms_ocall_sgx_connect_block_t* ms = SGX_CAST(ms_ocall_sgx_connect_block_t*, pms);
	ms->ms_retval = ocall_sgx_connect_block(ms->ms_socket, ms->ms_addr, ms->ms_port);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_accept(void* pms)
{
	ms_ocall_sgx_accept_t* ms = SGX_CAST(ms_ocall_sgx_accept_t*, pms);
	ms->ms_retval = ocall_sgx_accept(ms->ms_s, ms->ms_addr, ms->ms_addr_size, ms->ms_addrlen);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_accept_block(void* pms)
{
	ms_ocall_sgx_accept_block_t* ms = SGX_CAST(ms_ocall_sgx_accept_block_t*, pms);
	ms->ms_retval = ocall_sgx_accept_block(ms->ms_s);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_shutdown(void* pms)
{
	ms_ocall_sgx_shutdown_t* ms = SGX_CAST(ms_ocall_sgx_shutdown_t*, pms);
	ms->ms_retval = ocall_sgx_shutdown(ms->ms_fd);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_read(void* pms)
{
	ms_ocall_sgx_read_t* ms = SGX_CAST(ms_ocall_sgx_read_t*, pms);
	ms->ms_retval = ocall_sgx_read(ms->ms_fd, ms->ms_buf, ms->ms_n);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_write(void* pms)
{
	ms_ocall_sgx_write_t* ms = SGX_CAST(ms_ocall_sgx_write_t*, pms);
	ms->ms_retval = ocall_sgx_write(ms->ms_fd, ms->ms_buf, ms->ms_n);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_close(void* pms)
{
	ms_ocall_sgx_close_t* ms = SGX_CAST(ms_ocall_sgx_close_t*, pms);
	ms->ms_retval = ocall_sgx_close(ms->ms_fd);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_getenv(void* pms)
{
	ms_ocall_sgx_getenv_t* ms = SGX_CAST(ms_ocall_sgx_getenv_t*, pms);
	ms->ms_retval = ocall_sgx_getenv(ms->ms_env, ms->ms_envlen, ms->ms_ret_str, ms->ms_ret_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_select(void* pms)
{
	ms_ocall_sgx_select_t* ms = SGX_CAST(ms_ocall_sgx_select_t*, pms);
	ms->ms_retval = ocall_sgx_select(ms->ms_high_sock);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_exit(void* pms)
{
	ms_ocall_sgx_exit_t* ms = SGX_CAST(ms_ocall_sgx_exit_t*, pms);
	ocall_sgx_exit(ms->ms_code);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_puts(void* pms)
{
	ms_ocall_sgx_puts_t* ms = SGX_CAST(ms_ocall_sgx_puts_t*, pms);
	ocall_sgx_puts(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_puts_e(void* pms)
{
	ms_ocall_sgx_puts_e_t* ms = SGX_CAST(ms_ocall_sgx_puts_e_t*, pms);
	ocall_sgx_puts_e(ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_flush_e(void* pms)
{
	ms_ocall_sgx_flush_e_t* ms = SGX_CAST(ms_ocall_sgx_flush_e_t*, pms);
	ocall_sgx_flush_e();
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_clock(void* pms)
{
	ms_ocall_sgx_clock_t* ms = SGX_CAST(ms_ocall_sgx_clock_t*, pms);
	ms->ms_retval = ocall_sgx_clock();
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_time(void* pms)
{
	ms_ocall_sgx_time_t* ms = SGX_CAST(ms_ocall_sgx_time_t*, pms);
	ms->ms_retval = ocall_sgx_time(ms->ms_timep, ms->ms_t_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_localtime(void* pms)
{
	ms_ocall_sgx_localtime_t* ms = SGX_CAST(ms_ocall_sgx_localtime_t*, pms);
	ms->ms_retval = ocall_sgx_localtime(ms->ms_timep, ms->ms_t_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_sgx_gmtime_r(void* pms)
{
	ms_ocall_sgx_gmtime_r_t* ms = SGX_CAST(ms_ocall_sgx_gmtime_r_t*, pms);
	ms->ms_retval = ocall_sgx_gmtime_r(ms->ms_timep, ms->ms_t_len, ms->ms_tmp, ms->ms_tmp_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_log(void* pms)
{
	ms_ocall_log_t* ms = SGX_CAST(ms_ocall_log_t*, pms);
	ocall_log(ms->ms_file, ms->ms_line, ms->ms_str);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_build_addr(void* pms)
{
	ms_ocall_build_addr_t* ms = SGX_CAST(ms_ocall_build_addr_t*, pms);
	ms->ms_retval = ocall_build_addr(ms->ms_port, ms->ms_ip_addr);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_net_start_listen_socket(void* pms)
{
	ms_ocall_net_start_listen_socket_t* ms = SGX_CAST(ms_ocall_net_start_listen_socket_t*, pms);
	ms->ms_retval = ocall_net_start_listen_socket(ms->ms_server_addr, ms->ms_server_port);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_net_get_my_ipv4_addr(void* pms)
{
	ms_ocall_net_get_my_ipv4_addr_t* ms = SGX_CAST(ms_ocall_net_get_my_ipv4_addr_t*, pms);
	ocall_net_get_my_ipv4_addr(ms->ms_addr, ms->ms_sz);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_needs_read(void* pms)
{
	ms_ocall_needs_read_t* ms = SGX_CAST(ms_ocall_needs_read_t*, pms);
	ms->ms_retval = ocall_needs_read(ms->ms_sock);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_needs_write(void* pms)
{
	ms_ocall_needs_write_t* ms = SGX_CAST(ms_ocall_needs_write_t*, pms);
	ms->ms_retval = ocall_needs_write(ms->ms_sock);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_needs_except(void* pms)
{
	ms_ocall_needs_except_t* ms = SGX_CAST(ms_ocall_needs_except_t*, pms);
	ms->ms_retval = ocall_needs_except(ms->ms_sock);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_select_list(void* pms)
{
	ms_ocall_select_list_t* ms = SGX_CAST(ms_ocall_select_list_t*, pms);
	ms->ms_retval = ocall_select_list(ms->ms_list, ms->ms_list_sz);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_sgx_oc_cpuidex(void* pms)
{
	ms_sgx_oc_cpuidex_t* ms = SGX_CAST(ms_sgx_oc_cpuidex_t*, pms);
	sgx_oc_cpuidex(ms->ms_cpuinfo, ms->ms_leaf, ms->ms_subleaf);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_sgx_thread_wait_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_wait_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_wait_untrusted_event_ocall(ms->ms_self);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_sgx_thread_set_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_set_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_untrusted_event_ocall(ms->ms_waiter);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_sgx_thread_setwait_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_setwait_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_setwait_untrusted_events_ocall(ms->ms_waiter, ms->ms_self);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_sgx_thread_set_multiple_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_multiple_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_multiple_untrusted_events_ocall(ms->ms_waiters, ms->ms_total);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[41];
} ocall_table_enclave = {
	41,
	{
		(void*)enclave_ocall_start_enclave_report,
		(void*)enclave_ocall_sgx_olog,
		(void*)enclave_ocall_sgx_elog,
		(void*)enclave_ocall_sgx_eolog,
		(void*)enclave_ocall_sgx_gettimeofday,
		(void*)enclave_ocall_sgx_getsockopt,
		(void*)enclave_ocall_sgx_setsockopt,
		(void*)enclave_ocall_sgx_socket,
		(void*)enclave_ocall_sgx_listen,
		(void*)enclave_ocall_sgx_bind,
		(void*)enclave_ocall_sgx_connect,
		(void*)enclave_ocall_sgx_connect_block,
		(void*)enclave_ocall_sgx_accept,
		(void*)enclave_ocall_sgx_accept_block,
		(void*)enclave_ocall_sgx_shutdown,
		(void*)enclave_ocall_sgx_read,
		(void*)enclave_ocall_sgx_write,
		(void*)enclave_ocall_sgx_close,
		(void*)enclave_ocall_sgx_getenv,
		(void*)enclave_ocall_sgx_select,
		(void*)enclave_ocall_sgx_exit,
		(void*)enclave_ocall_sgx_puts,
		(void*)enclave_ocall_sgx_puts_e,
		(void*)enclave_ocall_sgx_flush_e,
		(void*)enclave_ocall_sgx_clock,
		(void*)enclave_ocall_sgx_time,
		(void*)enclave_ocall_sgx_localtime,
		(void*)enclave_ocall_sgx_gmtime_r,
		(void*)enclave_ocall_log,
		(void*)enclave_ocall_build_addr,
		(void*)enclave_ocall_net_start_listen_socket,
		(void*)enclave_ocall_net_get_my_ipv4_addr,
		(void*)enclave_ocall_needs_read,
		(void*)enclave_ocall_needs_write,
		(void*)enclave_ocall_needs_except,
		(void*)enclave_ocall_select_list,
		(void*)enclave_sgx_oc_cpuidex,
		(void*)enclave_sgx_thread_wait_untrusted_event_ocall,
		(void*)enclave_sgx_thread_set_untrusted_event_ocall,
		(void*)enclave_sgx_thread_setwait_untrusted_events_ocall,
		(void*)enclave_sgx_thread_set_multiple_untrusted_events_ocall,
	}
};
sgx_status_t restart_rollback_enclave_start(sgx_enclave_id_t eid, config_t conf, ssize_t my_idx, void* mmaped_addr, size_t addr_size, void* measure_buffer, size_t buffer_size)
{
	sgx_status_t status;
	ms_restart_rollback_enclave_start_t ms;
	ms.ms_conf = conf;
	ms.ms_my_idx = my_idx;
	ms.ms_mmaped_addr = mmaped_addr;
	ms.ms_addr_size = addr_size;
	ms.ms_measure_buffer = measure_buffer;
	ms.ms_buffer_size = buffer_size;
	status = sgx_ecall(eid, 0, &ocall_table_enclave, &ms);
	return status;
}

