#include "enclave_t.h"

#include "sgx_lfence.h" /* for sgx_lfence */
#include "sgx_trts.h"   /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h>     /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz)                          \
    do {                                                     \
        if (!(ptr) || !sgx_is_outside_enclave((ptr), (siz))) \
            return SGX_ERROR_INVALID_PARAMETER;              \
    } while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz)                      \
    do {                                                    \
        if ((ptr) && !sgx_is_outside_enclave((ptr), (siz))) \
            return SGX_ERROR_INVALID_PARAMETER;             \
    } while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz)                    \
    do {                                                   \
        if ((ptr) && !sgx_is_within_enclave((ptr), (siz))) \
            return SGX_ERROR_INVALID_PARAMETER;            \
    } while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (((a) += (b)) < (b))

typedef struct ms_pbft_enclave_start_t {
    config_t ms_conf;
    ssize_t ms_my_idx;
    void* ms_mmaped_addr;
    size_t ms_addr_size;
    void* ms_measure_buffer;
    size_t ms_buffer_size;
} ms_pbft_enclave_start_t;

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

static sgx_status_t SGX_CDECL sgx_pbft_enclave_start(void* pms) {
    CHECK_REF_POINTER(pms, sizeof(ms_pbft_enclave_start_t));
    //
    // fence after pointer checks
    //
    sgx_lfence();
    ms_pbft_enclave_start_t* ms = SGX_CAST(ms_pbft_enclave_start_t*, pms);
    sgx_status_t status = SGX_SUCCESS;
    void* _tmp_mmaped_addr = ms->ms_mmaped_addr;
    void* _tmp_measure_buffer = ms->ms_measure_buffer;

    pbft_enclave_start(ms->ms_conf, ms->ms_my_idx, _tmp_mmaped_addr,
                       ms->ms_addr_size, _tmp_measure_buffer,
                       ms->ms_buffer_size);

    return status;
}

SGX_EXTERNC const struct {
    size_t nr_ecall;
    struct {
        void* ecall_addr;
        uint8_t is_priv;
        uint8_t is_switchless;
    } ecall_table[1];
} g_ecall_table = {1,
                   {
                       {(void*)(uintptr_t)sgx_pbft_enclave_start, 0, 0},
                   }};

SGX_EXTERNC const struct {
    size_t nr_ocall;
    uint8_t entry_table[41][1];
} g_dyn_entry_table = {41,
                       {
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                           {
                               0,
                           },
                       }};

sgx_status_t SGX_CDECL ocall_start_enclave_report(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_start_enclave_report_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_start_enclave_report_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_start_enclave_report_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_start_enclave_report_t));
    ocalloc_size -= sizeof(ms_ocall_start_enclave_report_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(0, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_olog(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_sgx_olog_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_olog_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_olog_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_olog_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_olog_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(1, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_elog(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_sgx_elog_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_elog_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_elog_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_elog_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_elog_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(2, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_eolog(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_sgx_eolog_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_eolog_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_eolog_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_eolog_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_eolog_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(3, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_gettimeofday(int* retval, void* tv,
                                              int tv_size) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_tv = tv_size;

    ms_ocall_sgx_gettimeofday_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_gettimeofday_t);
    void* __tmp = NULL;

    void* __tmp_tv = NULL;

    CHECK_ENCLAVE_POINTER(tv, _len_tv);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (tv != NULL) ? _len_tv : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_gettimeofday_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_gettimeofday_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_gettimeofday_t);

    if (tv != NULL) {
        ms->ms_tv = (void*)__tmp;
        __tmp_tv = __tmp;
        if (memcpy_s(__tmp, ocalloc_size, tv, _len_tv)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_tv);
        ocalloc_size -= _len_tv;
    } else {
        ms->ms_tv = NULL;
    }

    ms->ms_tv_size = tv_size;
    status = sgx_ocall(4, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (tv) {
            if (memcpy_s((void*)tv, _len_tv, __tmp_tv, _len_tv)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_getsockopt(int* retval, int s, int level,
                                            int optname, char* optval,
                                            int optval_len, int* optlen) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_optval = optval_len;
    size_t _len_optlen = 4;

    ms_ocall_sgx_getsockopt_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_getsockopt_t);
    void* __tmp = NULL;

    void* __tmp_optval = NULL;
    void* __tmp_optlen = NULL;

    CHECK_ENCLAVE_POINTER(optval, _len_optval);
    CHECK_ENCLAVE_POINTER(optlen, _len_optlen);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (optval != NULL) ? _len_optval : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (optlen != NULL) ? _len_optlen : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_getsockopt_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_getsockopt_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_getsockopt_t);

    ms->ms_s = s;
    ms->ms_level = level;
    ms->ms_optname = optname;
    if (optval != NULL) {
        ms->ms_optval = (char*)__tmp;
        __tmp_optval = __tmp;
        if (_len_optval % sizeof(*optval) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        memset(__tmp_optval, 0, _len_optval);
        __tmp = (void*)((size_t)__tmp + _len_optval);
        ocalloc_size -= _len_optval;
    } else {
        ms->ms_optval = NULL;
    }

    ms->ms_optval_len = optval_len;
    if (optlen != NULL) {
        ms->ms_optlen = (int*)__tmp;
        __tmp_optlen = __tmp;
        if (_len_optlen % sizeof(*optlen) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, optlen, _len_optlen)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_optlen);
        ocalloc_size -= _len_optlen;
    } else {
        ms->ms_optlen = NULL;
    }

    status = sgx_ocall(5, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (optval) {
            if (memcpy_s((void*)optval, _len_optval, __tmp_optval,
                         _len_optval)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        if (optlen) {
            if (memcpy_s((void*)optlen, _len_optlen, __tmp_optlen,
                         _len_optlen)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_setsockopt(int* retval, int s, int level,
                                            int optname, const void* optval,
                                            int optlen) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_optval = optlen;

    ms_ocall_sgx_setsockopt_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_setsockopt_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(optval, _len_optval);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (optval != NULL) ? _len_optval : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_setsockopt_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_setsockopt_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_setsockopt_t);

    ms->ms_s = s;
    ms->ms_level = level;
    ms->ms_optname = optname;
    if (optval != NULL) {
        ms->ms_optval = (const void*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, optval, _len_optval)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_optval);
        ocalloc_size -= _len_optval;
    } else {
        ms->ms_optval = NULL;
    }

    ms->ms_optlen = optlen;
    status = sgx_ocall(6, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_socket(int* retval, int af, int type,
                                        int protocol) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_socket_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_socket_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_socket_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_socket_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_socket_t);

    ms->ms_af = af;
    ms->ms_type = type;
    ms->ms_protocol = protocol;
    status = sgx_ocall(7, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_listen(int* retval, int s, int backlog) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_listen_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_listen_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_listen_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_listen_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_listen_t);

    ms->ms_s = s;
    ms->ms_backlog = backlog;
    status = sgx_ocall(8, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_bind(int* retval, int s, const void* addr,
                                      int addr_size) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_addr = addr_size;

    ms_ocall_sgx_bind_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_bind_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(addr, _len_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_bind_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_bind_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_bind_t);

    ms->ms_s = s;
    if (addr != NULL) {
        ms->ms_addr = (const void*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, addr, _len_addr)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_addr);
        ocalloc_size -= _len_addr;
    } else {
        ms->ms_addr = NULL;
    }

    ms->ms_addr_size = addr_size;
    status = sgx_ocall(9, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_connect(int* retval, int s, const void* addr,
                                         int addrlen) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_addr = addrlen;

    ms_ocall_sgx_connect_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_connect_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(addr, _len_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_connect_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_connect_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_connect_t);

    ms->ms_s = s;
    if (addr != NULL) {
        ms->ms_addr = (const void*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, addr, _len_addr)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_addr);
        ocalloc_size -= _len_addr;
    } else {
        ms->ms_addr = NULL;
    }

    ms->ms_addrlen = addrlen;
    status = sgx_ocall(10, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_connect_block(int* retval, int* socket,
                                               const char* addr,
                                               uint16_t port) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_socket = 1 * sizeof(int);
    size_t _len_addr = addr ? strlen(addr) + 1 : 0;

    ms_ocall_sgx_connect_block_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_connect_block_t);
    void* __tmp = NULL;

    void* __tmp_socket = NULL;

    CHECK_ENCLAVE_POINTER(socket, _len_socket);
    CHECK_ENCLAVE_POINTER(addr, _len_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (socket != NULL) ? _len_socket : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_connect_block_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_connect_block_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_connect_block_t);

    if (socket != NULL) {
        ms->ms_socket = (int*)__tmp;
        __tmp_socket = __tmp;
        if (_len_socket % sizeof(*socket) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        memset(__tmp_socket, 0, _len_socket);
        __tmp = (void*)((size_t)__tmp + _len_socket);
        ocalloc_size -= _len_socket;
    } else {
        ms->ms_socket = NULL;
    }

    if (addr != NULL) {
        ms->ms_addr = (const char*)__tmp;
        if (_len_addr % sizeof(*addr) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, addr, _len_addr)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_addr);
        ocalloc_size -= _len_addr;
    } else {
        ms->ms_addr = NULL;
    }

    ms->ms_port = port;
    status = sgx_ocall(11, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (socket) {
            if (memcpy_s((void*)socket, _len_socket, __tmp_socket,
                         _len_socket)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_accept(int* retval, int s, void* addr,
                                        int addr_size, int* addrlen) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_addr = addr_size;
    size_t _len_addrlen = 4;

    ms_ocall_sgx_accept_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_accept_t);
    void* __tmp = NULL;

    void* __tmp_addr = NULL;
    void* __tmp_addrlen = NULL;

    CHECK_ENCLAVE_POINTER(addr, _len_addr);
    CHECK_ENCLAVE_POINTER(addrlen, _len_addrlen);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addrlen != NULL) ? _len_addrlen : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_accept_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_accept_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_accept_t);

    ms->ms_s = s;
    if (addr != NULL) {
        ms->ms_addr = (void*)__tmp;
        __tmp_addr = __tmp;
        memset(__tmp_addr, 0, _len_addr);
        __tmp = (void*)((size_t)__tmp + _len_addr);
        ocalloc_size -= _len_addr;
    } else {
        ms->ms_addr = NULL;
    }

    ms->ms_addr_size = addr_size;
    if (addrlen != NULL) {
        ms->ms_addrlen = (int*)__tmp;
        __tmp_addrlen = __tmp;
        if (_len_addrlen % sizeof(*addrlen) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, addrlen, _len_addrlen)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_addrlen);
        ocalloc_size -= _len_addrlen;
    } else {
        ms->ms_addrlen = NULL;
    }

    status = sgx_ocall(12, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (addr) {
            if (memcpy_s((void*)addr, _len_addr, __tmp_addr, _len_addr)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        if (addrlen) {
            if (memcpy_s((void*)addrlen, _len_addrlen, __tmp_addrlen,
                         _len_addrlen)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_accept_block(int* retval, int s) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_accept_block_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_accept_block_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_accept_block_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_accept_block_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_accept_block_t);

    ms->ms_s = s;
    status = sgx_ocall(13, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_shutdown(int* retval, int fd) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_shutdown_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_shutdown_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_shutdown_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_shutdown_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_shutdown_t);

    ms->ms_fd = fd;
    status = sgx_ocall(14, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_read(int* retval, int fd, void* buf, int n) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_buf = n;

    ms_ocall_sgx_read_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_read_t);
    void* __tmp = NULL;

    void* __tmp_buf = NULL;

    CHECK_ENCLAVE_POINTER(buf, _len_buf);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_read_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_read_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_read_t);

    ms->ms_fd = fd;
    if (buf != NULL) {
        ms->ms_buf = (void*)__tmp;
        __tmp_buf = __tmp;
        memset(__tmp_buf, 0, _len_buf);
        __tmp = (void*)((size_t)__tmp + _len_buf);
        ocalloc_size -= _len_buf;
    } else {
        ms->ms_buf = NULL;
    }

    ms->ms_n = n;
    status = sgx_ocall(15, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (buf) {
            if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_write(int* retval, int fd, const void* buf,
                                       int n) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_buf = n;

    ms_ocall_sgx_write_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_write_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(buf, _len_buf);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_write_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_write_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_write_t);

    ms->ms_fd = fd;
    if (buf != NULL) {
        ms->ms_buf = (const void*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_buf);
        ocalloc_size -= _len_buf;
    } else {
        ms->ms_buf = NULL;
    }

    ms->ms_n = n;
    status = sgx_ocall(16, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_close(int* retval, int fd) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_close_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_close_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_close_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_close_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_close_t);

    ms->ms_fd = fd;
    status = sgx_ocall(17, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_getenv(int* retval, const char* env,
                                        int envlen, char* ret_str,
                                        int ret_len) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_env = envlen;
    size_t _len_ret_str = ret_len;

    ms_ocall_sgx_getenv_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_getenv_t);
    void* __tmp = NULL;

    void* __tmp_ret_str = NULL;

    CHECK_ENCLAVE_POINTER(env, _len_env);
    CHECK_ENCLAVE_POINTER(ret_str, _len_ret_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (env != NULL) ? _len_env : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ret_str != NULL) ? _len_ret_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_getenv_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_getenv_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_getenv_t);

    if (env != NULL) {
        ms->ms_env = (const char*)__tmp;
        if (_len_env % sizeof(*env) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, env, _len_env)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_env);
        ocalloc_size -= _len_env;
    } else {
        ms->ms_env = NULL;
    }

    ms->ms_envlen = envlen;
    if (ret_str != NULL) {
        ms->ms_ret_str = (char*)__tmp;
        __tmp_ret_str = __tmp;
        if (_len_ret_str % sizeof(*ret_str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        memset(__tmp_ret_str, 0, _len_ret_str);
        __tmp = (void*)((size_t)__tmp + _len_ret_str);
        ocalloc_size -= _len_ret_str;
    } else {
        ms->ms_ret_str = NULL;
    }

    ms->ms_ret_len = ret_len;
    status = sgx_ocall(18, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (ret_str) {
            if (memcpy_s((void*)ret_str, _len_ret_str, __tmp_ret_str,
                         _len_ret_str)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_select(int* retval, int high_sock) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_select_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_select_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_select_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_select_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_select_t);

    ms->ms_high_sock = high_sock;
    status = sgx_ocall(19, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_exit(int code) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_exit_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_exit_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_exit_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_exit_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_exit_t);

    ms->ms_code = code;
    status = sgx_ocall(20, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_puts(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_sgx_puts_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_puts_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_puts_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_puts_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_puts_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(21, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_puts_e(const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_sgx_puts_e_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_puts_e_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_puts_e_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_puts_e_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_puts_e_t);

    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(22, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_flush_e(void) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_flush_e_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_flush_e_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_flush_e_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_flush_e_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_flush_e_t);

    status = sgx_ocall(23, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_clock(long int* retval) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_sgx_clock_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_clock_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_clock_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_clock_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_clock_t);

    status = sgx_ocall(24, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_time(time_t* retval, time_t* timep,
                                      int t_len) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_timep = t_len;

    ms_ocall_sgx_time_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_time_t);
    void* __tmp = NULL;

    void* __tmp_timep = NULL;

    CHECK_ENCLAVE_POINTER(timep, _len_timep);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (timep != NULL) ? _len_timep : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_time_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_time_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_time_t);

    if (timep != NULL) {
        ms->ms_timep = (time_t*)__tmp;
        __tmp_timep = __tmp;
        memset(__tmp_timep, 0, _len_timep);
        __tmp = (void*)((size_t)__tmp + _len_timep);
        ocalloc_size -= _len_timep;
    } else {
        ms->ms_timep = NULL;
    }

    ms->ms_t_len = t_len;
    status = sgx_ocall(25, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (timep) {
            if (memcpy_s((void*)timep, _len_timep, __tmp_timep, _len_timep)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_localtime(struct tm** retval,
                                           const time_t* timep, int t_len) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_timep = t_len;

    ms_ocall_sgx_localtime_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_localtime_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(timep, _len_timep);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (timep != NULL) ? _len_timep : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_localtime_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_localtime_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_localtime_t);

    if (timep != NULL) {
        ms->ms_timep = (const time_t*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, timep, _len_timep)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_timep);
        ocalloc_size -= _len_timep;
    } else {
        ms->ms_timep = NULL;
    }

    ms->ms_t_len = t_len;
    status = sgx_ocall(26, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_sgx_gmtime_r(struct tm** retval,
                                          const time_t* timep, int t_len,
                                          struct tm* tmp, int tmp_len) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_timep = t_len;
    size_t _len_tmp = tmp_len;

    ms_ocall_sgx_gmtime_r_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_sgx_gmtime_r_t);
    void* __tmp = NULL;

    void* __tmp_tmp = NULL;

    CHECK_ENCLAVE_POINTER(timep, _len_timep);
    CHECK_ENCLAVE_POINTER(tmp, _len_tmp);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (timep != NULL) ? _len_timep : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (tmp != NULL) ? _len_tmp : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_sgx_gmtime_r_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_sgx_gmtime_r_t));
    ocalloc_size -= sizeof(ms_ocall_sgx_gmtime_r_t);

    if (timep != NULL) {
        ms->ms_timep = (const time_t*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, timep, _len_timep)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_timep);
        ocalloc_size -= _len_timep;
    } else {
        ms->ms_timep = NULL;
    }

    ms->ms_t_len = t_len;
    if (tmp != NULL) {
        ms->ms_tmp = (struct tm*)__tmp;
        __tmp_tmp = __tmp;
        memset(__tmp_tmp, 0, _len_tmp);
        __tmp = (void*)((size_t)__tmp + _len_tmp);
        ocalloc_size -= _len_tmp;
    } else {
        ms->ms_tmp = NULL;
    }

    ms->ms_tmp_len = tmp_len;
    status = sgx_ocall(27, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        if (tmp) {
            if (memcpy_s((void*)tmp, _len_tmp, __tmp_tmp, _len_tmp)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_log(const char* file, int line, const char* str) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_file = file ? strlen(file) + 1 : 0;
    size_t _len_str = str ? strlen(str) + 1 : 0;

    ms_ocall_log_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_log_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(file, _len_file);
    CHECK_ENCLAVE_POINTER(str, _len_str);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (file != NULL) ? _len_file : 0))
        return SGX_ERROR_INVALID_PARAMETER;
    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_log_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_log_t));
    ocalloc_size -= sizeof(ms_ocall_log_t);

    if (file != NULL) {
        ms->ms_file = (const char*)__tmp;
        if (_len_file % sizeof(*file) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, file, _len_file)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_file);
        ocalloc_size -= _len_file;
    } else {
        ms->ms_file = NULL;
    }

    ms->ms_line = line;
    if (str != NULL) {
        ms->ms_str = (const char*)__tmp;
        if (_len_str % sizeof(*str) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_str);
        ocalloc_size -= _len_str;
    } else {
        ms->ms_str = NULL;
    }

    status = sgx_ocall(28, ms);

    if (status == SGX_SUCCESS) {
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_build_addr(void** retval, uint16_t port,
                                        const char* ip_addr) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_ip_addr = ip_addr ? strlen(ip_addr) + 1 : 0;

    ms_ocall_build_addr_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_build_addr_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(ip_addr, _len_ip_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ip_addr != NULL) ? _len_ip_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_build_addr_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_build_addr_t));
    ocalloc_size -= sizeof(ms_ocall_build_addr_t);

    ms->ms_port = port;
    if (ip_addr != NULL) {
        ms->ms_ip_addr = (const char*)__tmp;
        if (_len_ip_addr % sizeof(*ip_addr) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, ip_addr, _len_ip_addr)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_ip_addr);
        ocalloc_size -= _len_ip_addr;
    } else {
        ms->ms_ip_addr = NULL;
    }

    status = sgx_ocall(29, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_net_start_listen_socket(int* retval,
                                                     const char* server_addr,
                                                     uint16_t server_port) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_server_addr = server_addr ? strlen(server_addr) + 1 : 0;

    ms_ocall_net_start_listen_socket_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_net_start_listen_socket_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(server_addr, _len_server_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size,
                            (server_addr != NULL) ? _len_server_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_net_start_listen_socket_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_net_start_listen_socket_t));
    ocalloc_size -= sizeof(ms_ocall_net_start_listen_socket_t);

    if (server_addr != NULL) {
        ms->ms_server_addr = (const char*)__tmp;
        if (_len_server_addr % sizeof(*server_addr) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, server_addr, _len_server_addr)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_server_addr);
        ocalloc_size -= _len_server_addr;
    } else {
        ms->ms_server_addr = NULL;
    }

    ms->ms_server_port = server_port;
    status = sgx_ocall(30, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_net_get_my_ipv4_addr(char* addr, size_t sz) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_addr = sz;

    ms_ocall_net_get_my_ipv4_addr_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_net_get_my_ipv4_addr_t);
    void* __tmp = NULL;

    void* __tmp_addr = NULL;

    CHECK_ENCLAVE_POINTER(addr, _len_addr);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_net_get_my_ipv4_addr_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_net_get_my_ipv4_addr_t));
    ocalloc_size -= sizeof(ms_ocall_net_get_my_ipv4_addr_t);

    if (addr != NULL) {
        ms->ms_addr = (char*)__tmp;
        __tmp_addr = __tmp;
        if (_len_addr % sizeof(*addr) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        memset(__tmp_addr, 0, _len_addr);
        __tmp = (void*)((size_t)__tmp + _len_addr);
        ocalloc_size -= _len_addr;
    } else {
        ms->ms_addr = NULL;
    }

    ms->ms_sz = sz;
    status = sgx_ocall(31, ms);

    if (status == SGX_SUCCESS) {
        if (addr) {
            if (memcpy_s((void*)addr, _len_addr, __tmp_addr, _len_addr)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_needs_read(int* retval, int sock) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_needs_read_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_needs_read_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_needs_read_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_needs_read_t));
    ocalloc_size -= sizeof(ms_ocall_needs_read_t);

    ms->ms_sock = sock;
    status = sgx_ocall(32, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_needs_write(int* retval, int sock) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_needs_write_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_needs_write_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_needs_write_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_needs_write_t));
    ocalloc_size -= sizeof(ms_ocall_needs_write_t);

    ms->ms_sock = sock;
    status = sgx_ocall(33, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_needs_except(int* retval, int sock) {
    sgx_status_t status = SGX_SUCCESS;

    ms_ocall_needs_except_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_needs_except_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_needs_except_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_needs_except_t));
    ocalloc_size -= sizeof(ms_ocall_needs_except_t);

    ms->ms_sock = sock;
    status = sgx_ocall(34, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL ocall_select_list(int* retval, selected_t* list,
                                         ssize_t list_sz) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_list = list_sz * sizeof(selected_t);

    ms_ocall_select_list_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_ocall_select_list_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(list, _len_list);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (list != NULL) ? _len_list : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_ocall_select_list_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_ocall_select_list_t));
    ocalloc_size -= sizeof(ms_ocall_select_list_t);

    if (list != NULL) {
        ms->ms_list = (selected_t*)__tmp;
        if (memcpy_s(__tmp, ocalloc_size, list, _len_list)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_list);
        ocalloc_size -= _len_list;
    } else {
        ms->ms_list = NULL;
    }

    ms->ms_list_sz = list_sz;
    status = sgx_ocall(35, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
        errno = ms->ocall_errno;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_cpuinfo = 4 * sizeof(int);

    ms_sgx_oc_cpuidex_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_sgx_oc_cpuidex_t);
    void* __tmp = NULL;

    void* __tmp_cpuinfo = NULL;

    CHECK_ENCLAVE_POINTER(cpuinfo, _len_cpuinfo);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (cpuinfo != NULL) ? _len_cpuinfo : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_sgx_oc_cpuidex_t*)__tmp;
    __tmp = (void*)((size_t)__tmp + sizeof(ms_sgx_oc_cpuidex_t));
    ocalloc_size -= sizeof(ms_sgx_oc_cpuidex_t);

    if (cpuinfo != NULL) {
        ms->ms_cpuinfo = (int*)__tmp;
        __tmp_cpuinfo = __tmp;
        if (_len_cpuinfo % sizeof(*cpuinfo) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        memset(__tmp_cpuinfo, 0, _len_cpuinfo);
        __tmp = (void*)((size_t)__tmp + _len_cpuinfo);
        ocalloc_size -= _len_cpuinfo;
    } else {
        ms->ms_cpuinfo = NULL;
    }

    ms->ms_leaf = leaf;
    ms->ms_subleaf = subleaf;
    status = sgx_ocall(36, ms);

    if (status == SGX_SUCCESS) {
        if (cpuinfo) {
            if (memcpy_s((void*)cpuinfo, _len_cpuinfo, __tmp_cpuinfo,
                         _len_cpuinfo)) {
                sgx_ocfree();
                return SGX_ERROR_UNEXPECTED;
            }
        }
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval,
                                                             const void* self) {
    sgx_status_t status = SGX_SUCCESS;

    ms_sgx_thread_wait_untrusted_event_ocall_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_sgx_thread_wait_untrusted_event_ocall_t*)__tmp;
    __tmp = (void*)((size_t)__tmp +
                    sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t));
    ocalloc_size -= sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);

    ms->ms_self = self;
    status = sgx_ocall(37, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL
sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter) {
    sgx_status_t status = SGX_SUCCESS;

    ms_sgx_thread_set_untrusted_event_ocall_t* ms = NULL;
    size_t ocalloc_size = sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_sgx_thread_set_untrusted_event_ocall_t*)__tmp;
    __tmp = (void*)((size_t)__tmp +
                    sizeof(ms_sgx_thread_set_untrusted_event_ocall_t));
    ocalloc_size -= sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);

    ms->ms_waiter = waiter;
    status = sgx_ocall(38, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(
    int* retval, const void* waiter, const void* self) {
    sgx_status_t status = SGX_SUCCESS;

    ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = NULL;
    size_t ocalloc_size =
        sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);
    void* __tmp = NULL;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_sgx_thread_setwait_untrusted_events_ocall_t*)__tmp;
    __tmp = (void*)((size_t)__tmp +
                    sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t));
    ocalloc_size -= sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);

    ms->ms_waiter = waiter;
    ms->ms_self = self;
    status = sgx_ocall(39, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
    }
    sgx_ocfree();
    return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(
    int* retval, const void** waiters, size_t total) {
    sgx_status_t status = SGX_SUCCESS;
    size_t _len_waiters = total * sizeof(void*);

    ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = NULL;
    size_t ocalloc_size =
        sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);
    void* __tmp = NULL;

    CHECK_ENCLAVE_POINTER(waiters, _len_waiters);

    if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (waiters != NULL) ? _len_waiters : 0))
        return SGX_ERROR_INVALID_PARAMETER;

    __tmp = sgx_ocalloc(ocalloc_size);
    if (__tmp == NULL) {
        sgx_ocfree();
        return SGX_ERROR_UNEXPECTED;
    }
    ms = (ms_sgx_thread_set_multiple_untrusted_events_ocall_t*)__tmp;
    __tmp =
        (void*)((size_t)__tmp +
                sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t));
    ocalloc_size -= sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);

    if (waiters != NULL) {
        ms->ms_waiters = (const void**)__tmp;
        if (_len_waiters % sizeof(*waiters) != 0) {
            sgx_ocfree();
            return SGX_ERROR_INVALID_PARAMETER;
        }
        if (memcpy_s(__tmp, ocalloc_size, waiters, _len_waiters)) {
            sgx_ocfree();
            return SGX_ERROR_UNEXPECTED;
        }
        __tmp = (void*)((size_t)__tmp + _len_waiters);
        ocalloc_size -= _len_waiters;
    } else {
        ms->ms_waiters = NULL;
    }

    ms->ms_total = total;
    status = sgx_ocall(40, ms);

    if (status == SGX_SUCCESS) {
        if (retval) *retval = ms->ms_retval;
    }
    sgx_ocfree();
    return status;
}
