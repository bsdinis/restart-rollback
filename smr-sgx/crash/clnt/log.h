/*
 * useful macros (and small funcs)
 */
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// this can be used to tune the loggin on a per file basis
#ifndef log_level
#define log_level 0
#endif

#define ERROR(...)                                                    \
    {                                                                 \
        char buf[2048];                                               \
        snprintf(buf, 2048, __VA_ARGS__);                             \
        fprintf(stderr, "[%d] | [ERR] %s:%d: %s\n", \
                getpid(), __FILE__, __LINE__, buf);                   \
    }

#define INFO(...)                                                      \
    {                                                                  \
        char buf[2048];                                                \
        snprintf(buf, 2048, __VA_ARGS__);                              \
        fprintf(stderr, "[%d] | [INFO] %s:%d: %s\n", \
                getpid(), __FILE__, __LINE__, buf);                    \
    }

#define KILL(...)                                                      \
    {                                                                  \
        char buf[2048];                                                \
        snprintf(buf, 2048, __VA_ARGS__);                              \
        fprintf(stderr, "[%d] | [KILL] %s:%d: %s\n", \
                getpid(), __FILE__, __LINE__, buf);                    \
        exit(-1);                                                      \
    }

#if log_level > 0
#define LOG(...)                                                      \
    {                                                                 \
        char buf[2048];                                               \
        snprintf(buf, 2048, __VA_ARGS__);                             \
        fprintf(stderr, "[%d] | [LOG] %s:%d: %s\n", \
                getpid(), __FILE__, __LINE__, buf);                   \
    }
#else
#define LOG(...)
#endif

#if log_level > 1
#define FINE(...)                                                      \
    {                                                                  \
        char buf[2048];                                                \
        snprintf(buf, 2048, __VA_ARGS__);                              \
        fprintf(stdout, "[%d] | [FINE] %s:%d: %s\n", \
                getpid(), __FILE__, __LINE__, buf);                    \
    }
#else
#define FINE(...)
#endif

#if defined(__cplusplus)
}
#endif
