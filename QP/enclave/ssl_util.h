/***
 * ssl_util.h
 *
 * utility wrappers for openssl
 */

#ifndef __SSL_UTIL_HEADER__
#define __SSL_UTIL_HEADER__

#if defined(__cplusplus)
extern "C" {
#endif

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdio.h>

namespace epidemics {
namespace basicQP {
namespace ssl {

int init_client_ssl_ctx(SSL_CTX **);
int init_server_ssl_ctx(SSL_CTX **);

int close_ssl_ctx(SSL_CTX *);

int load_certificates(SSL_CTX *, const char *const cert, const char *const key);

}  // namespace ssl
}  // namespace basicQP
}  // namespace epidemics

#if defined(__cplusplus)
}
#endif
#endif  //__SSL_UTIL_HEADER__
