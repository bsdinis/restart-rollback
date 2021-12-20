/*
 * logging functions
 */

#ifndef __MACROS_HEADER__
#define __MACROS_HEADER__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "enclave_t.h"

void _priv_ereport(char const *file, int lineno, char const *prefix,
                   char const *fmt, ...);

void _priv_oreport(char const *file, int lineno, char const *prefix,
                   char const *fmt, ...);

// this can be used to tune the loggin on a per file basis
#ifndef log_level
#define log_level 2
#endif

#define ERROR(...) _priv_ereport(__FILE__, __LINE__, "ERR", __VA_ARGS__)
#define INFO(...) _priv_ereport(__FILE__, __LINE__, "INFO", __VA_ARGS__)

#if log_level > 0
#define LOG(...) _priv_ereport(__FILE__, __LINE__, "LOG", __VA_ARGS__)
#else
#define LOG(...)
#endif

#if log_level > 1
#define FINE(...) _priv_ereport(__FILE__, __LINE__, "FINE", __VA_ARGS__)
#else
#define FINE(...)
#endif

#define KILL(...)                                               \
    {                                                           \
        _priv_ereport(__FILE__, __LINE__, "KILL", __VA_ARGS__); \
        ocall_sgx_exit(-1);                                     \
    }

#if defined(__cplusplus)
}
#endif

#endif  //__MACROS_HEADER__
