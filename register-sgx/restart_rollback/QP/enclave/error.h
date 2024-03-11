/***
 * error.h
 *
 * make error handling easier
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include "enclave_t.h"

void __sgx_perror(const char *func_name, sgx_status_t);

#define sgx_perror(errcode) __sgx_perror(__func__, errcode)

#if defined(__cplusplus)
}
#endif
