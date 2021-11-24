/**
 * setup.h
 *
 * setup routines for gatekeeper
 *
 * should be called onde and without parameters (ergo, this header is very
 * simple)
 */

#ifndef __SETUP_HEADER__
#define __SETUP_HEADER__

#include <stdint.h>
#include <stdio.h>
#include "config.h"

#include <vector>
#include "peer.h"

namespace epidemics {
namespace basicQP {
namespace setup {

void setup(config_t* conf, ssize_t idx);
void close();

SSL_CTX* ssl_ctx();
int listen_sock();
bool closed();

}  // namespace setup
}  // namespace basicQP
}  // namespace epidemics

#endif  //__SETUP_HEADER__
