/**
 * config.h
 *
 * reads the config file provided
 * fills up config with the gatekeepers IP address
 *
 * accepts the following lines
 *
 * # comment
 * min_addr max_addr quorum_size
 * ip_addr port
 * ip_addr
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct config_node_t {
    char *	addr; // ipv4
    int		port; // positive
} config_node_t;

typedef struct config_t {
    size_t size;
    config_node_t * nodes;
} config_t;

int  config_parse(config_t * const conf, char const * filename);
void config_free(config_t * const conf);

#if defined(__cplusplus)
}
#endif

