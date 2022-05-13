#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct selected_t {
  int     sockfd;
  int     wr;
} selected_t;

#if defined(__cplusplus)
constexpr size_t REGISTER_SIZE = 128;
#else
#define REGISTER_SIZE 128
#endif

#if defined(__cplusplus)
}
#endif
