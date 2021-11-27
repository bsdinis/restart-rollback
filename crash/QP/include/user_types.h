#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// timestamp_t
#ifndef _struct_timestamp_t_definition_
#define _struct_timestamp_t_definition_
typedef struct timestamp_t
{
  uint64_t 	epoch;
  uint64_t 	seqno;
  uint64_t 	idx;
  int 	stable;
} timestamp_t;
#endif // _struct_timestamp_t_definition_

typedef uint64_t clnt_id_t;

#ifndef _struct_blob_t_definition_
#define _struct_blob_t_definition_
typedef struct blob_t
{
  uint8_t   enc_key[32];
  uint8_t   enc_iv[16];
  uint8_t   enc_data_hash[32];
  uint64_t  pol_id;
  char  ustor_name[424];
} blob_t;
#endif

#ifndef _struct_write_resp_definition_
#define _struct_write_resp_definition_
typedef struct write_resp {
  uint8_t ack;
  uint8_t r;
  blob_t blob;
  timestamp_t ts;
} write_resp;
#endif


typedef struct selected_t {
  int     sockfd;
  int     wr;
} selected_t;


#if defined(__cplusplus)
}
#endif
