// implemtation

#include "log.h"
#include "enclave_t.h"

#include <stdio.h>

void _priv_ereport(char const *file, int lineno, char const *prefix,
                   const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ + 64] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    snprintf(buf2, BUFSIZ + 64, "[%s] %s:%d: %s\n", prefix, file, lineno, buf);
    ocall_sgx_eolog(buf2);
}

void _priv_oreport(char const *file, int lineno, char const *prefix,
                   const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ + 64] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    snprintf(buf2, BUFSIZ + 64, "[%s] %s:%d: %s\n", prefix, file, lineno, buf);
    ocall_sgx_olog(buf2);
}
