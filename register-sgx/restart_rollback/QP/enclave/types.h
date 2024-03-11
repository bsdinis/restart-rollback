#ifndef TYPES_H
#define TYPES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

typedef unsigned short int sa_family_t;

#define __SOCKADDR_COMMON(sa_prefix) sa_family_t sa_prefix##family

#ifndef __BITS_SOCKET_H
#define __BITS_SOCKET_H
struct sockaddr {
    __SOCKADDR_COMMON(sa_); /* Common data: address family and length.  */
    char sa_data[14];       /* Address data.  */
};
#endif

typedef int pid_t;
typedef unsigned int uid_t;
typedef __time_t time_t;

typedef long int __suseconds_t;

#ifndef __timeval_defined
#define __timeval_defined
#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL 1
struct timeval {
    __time_t tv_sec;       /* Seconds.  */
    __suseconds_t tv_usec; /* Microseconds.  */
};
#endif
#endif

#ifndef AF_INET
#define AF_INET 2
#endif

#ifndef htons
#define htons(n) \
    (((((unsigned short)(n)&0xFF)) << 8) | (((unsigned short)(n)&0xFF00) >> 8))
#endif

#ifndef ntohs
#define ntohs(n) \
    (((((unsigned short)(n)&0xFF)) << 8) | (((unsigned short)(n)&0xFF00) >> 8))
#endif

#ifndef htonl
#define htonl(n)                              \
    (((((unsigned long)(n)&0xFF)) << 24) |    \
     ((((unsigned long)(n)&0xFF00)) << 8) |   \
     ((((unsigned long)(n)&0xFF0000)) >> 8) | \
     ((((unsigned long)(n)&0xFF000000)) >> 24))
#endif

#ifndef ntohl
#define ntohl(n)                              \
    (((((unsigned long)(n)&0xFF)) << 24) |    \
     ((((unsigned long)(n)&0xFF00)) << 8) |   \
     ((((unsigned long)(n)&0xFF0000)) >> 8) | \
     ((((unsigned long)(n)&0xFF000000)) >> 24))
#endif

typedef uint32_t in_addr_t;
#ifndef INADDR_ANY
#define INADDR_ANY ((in_addr_t)0x00000000)
#endif

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#ifndef __BITS_SOCKET_H
#define __BITS_SOCKET_H
struct in_addr {
    in_addr_t s_addr;
};
#endif

#ifndef __SOCKADDR_COMMON_SIZE
#define __SOCKADDR_COMMON_SIZE (sizeof(unsigned short int))
#endif

#if __SOCK_SIZE == 16
typedef struct sockaddr_in {
    __SOCKADDR_COMMON(sin_);
    in_port_t sin_port;      /* Port number.  */
    struct in_addr sin_addr; /* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof(struct sockaddr) - __SOCKADDR_COMMON_SIZE -
                           sizeof(in_port_t) - sizeof(struct in_addr)];
} sockaddr_in;
#else
typedef struct sockaddr_in sockaddr_in;
#endif

#ifndef SOCK_STREAM
#define SOCK_STREAM 1
#endif

#ifndef SOL_SOCKET
#define SOL_SOCKET 1
#endif

#ifndef SO_REUSEADDR
#define SO_REUSEADDR 2
#endif

#ifndef __FILE_defined
#define __FILE_defined
typedef struct _IO_FILE FILE;
#endif

// For open64 need to define this
// typedef __ssize_t ssize_t;
// typedef __gid_t gid_t;

#if defined(__cplusplus)
}
#endif
#endif
