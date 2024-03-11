/*
 * useful macros (and small funcs)
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// this can be used to tune the loggin on a per file basis
#ifndef log_level
#define log_level 0
#endif

extern ssize_t my_idx;

#define ERROR(...)                                                    \
    {                                                                 \
        char buf[2048];                                               \
        snprintf(buf, 2048, __VA_ARGS__);                             \
        fprintf(stderr, "[APP] %ld | %d | [ERR] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                   \
        fprintf(stdout, "[APP] %ld | %d | [ERR] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                   \
    }

#define INFO(...)                                                      \
    {                                                                  \
        char buf[2048];                                                \
        snprintf(buf, 2048, __VA_ARGS__);                              \
        fprintf(stderr, "[APP] %ld | %d | [INFO] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                    \
        fprintf(stdout, "[APP] %ld | %d | [INFO] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                    \
    }

#define KILL(...)                                                      \
    {                                                                  \
        char buf[2048];                                                \
        snprintf(buf, 2048, __VA_ARGS__);                              \
        fprintf(stderr, "[APP] %ld | %d | [KILL] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                    \
        fprintf(stdout, "[APP] %ld | %d | [KILL] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                    \
        exit(-1);                                                      \
    }

#if log_level > 0
#define LOG(...)                                                      \
    {                                                                 \
        char buf[2048];                                               \
        snprintf(buf, 2048, __VA_ARGS__);                             \
        fprintf(stdout, "[APP] %ld | %d | [LOG] %s:%d: %s\n", my_idx, \
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
        fprintf(stdout, "[APP] %ld | %d | [FINE] %s:%d: %s\n", my_idx, \
                getpid(), __FILE__, __LINE__, buf);                    \
    }
#else
#define FINE(...)
#endif
