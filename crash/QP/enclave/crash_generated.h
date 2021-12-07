// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_
#define FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_

#include "flatbuffers/flatbuffers.h"

namespace paxos_sgx {
namespace crash {

enum ReqType {
  ReqType_client_fast_get = 0,
  ReqType_client_operation_get = 1,
  ReqType_replica_fast_get = 2,
  ReqType_replica_propose = 3,
  ReqType_replica_accept = 4,
  ReqType_ping = 5,
  ReqType_close = 6,
  ReqType_MIN = ReqType_client_fast_get,
  ReqType_MAX = ReqType_close
};

inline const ReqType (&EnumValuesReqType())[7] {
  static const ReqType values[] = {
    ReqType_client_fast_get,
    ReqType_client_operation_get,
    ReqType_replica_fast_get,
    ReqType_replica_propose,
    ReqType_replica_accept,
    ReqType_ping,
    ReqType_close
  };
  return values;
}

inline const char * const *EnumNamesReqType() {
  static const char * const names[8] = {
    "client_fast_get",
    "client_operation_get",
    "replica_fast_get",
    "replica_propose",
    "replica_accept",
    "ping",
    "close",
    nullptr
  };
  return names;
}

inline const char *EnumNameReqType(ReqType e) {
  if (flatbuffers::IsOutRange(e, ReqType_client_fast_get, ReqType_close)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesReqType()[index];
}

}  // namespace crash
}  // namespace paxos_sgx

#endif  // FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_
