// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_CRASHRESP_PAXOS_SGX_CRASH_H_
#define FLATBUFFERS_GENERATED_CRASHRESP_PAXOS_SGX_CRASH_H_

#include "flatbuffers/flatbuffers.h"

#include "crash_generated.h"

namespace paxos_sgx {
namespace crash {

struct ClientGetResult;
struct ClientGetResultBuilder;

struct ReplicaGetResult;
struct ReplicaGetResultBuilder;

struct PingResult;
struct PingResultBuilder;

struct BasicResponse;
struct BasicResponseBuilder;

enum Result {
  Result_NONE = 0,
  Result_ClientGetResult = 1,
  Result_ReplicaGetResult = 2,
  Result_PingResult = 3,
  Result_MIN = Result_NONE,
  Result_MAX = Result_PingResult
};

inline const Result (&EnumValuesResult())[4] {
  static const Result values[] = {
    Result_NONE,
    Result_ClientGetResult,
    Result_ReplicaGetResult,
    Result_PingResult
  };
  return values;
}

inline const char * const *EnumNamesResult() {
  static const char * const names[5] = {
    "NONE",
    "ClientGetResult",
    "ReplicaGetResult",
    "PingResult",
    nullptr
  };
  return names;
}

inline const char *EnumNameResult(Result e) {
  if (flatbuffers::IsOutRange(e, Result_NONE, Result_PingResult)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesResult()[index];
}

template<typename T> struct ResultTraits {
  static const Result enum_value = Result_NONE;
};

template<> struct ResultTraits<paxos_sgx::crash::ClientGetResult> {
  static const Result enum_value = Result_ClientGetResult;
};

template<> struct ResultTraits<paxos_sgx::crash::ReplicaGetResult> {
  static const Result enum_value = Result_ReplicaGetResult;
};

template<> struct ResultTraits<paxos_sgx::crash::PingResult> {
  static const Result enum_value = Result_PingResult;
};

bool VerifyResult(flatbuffers::Verifier &verifier, const void *obj, Result type);
bool VerifyResultVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct ClientGetResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ClientGetResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACCOUNT = 4,
    VT_AMOUNT = 6,
    VT_SUCCESS = 8
  };
  int64_t account() const {
    return GetField<int64_t>(VT_ACCOUNT, 0);
  }
  int64_t amount() const {
    return GetField<int64_t>(VT_AMOUNT, 0);
  }
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           VerifyField<int64_t>(verifier, VT_AMOUNT) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS) &&
           verifier.EndTable();
  }
};

struct ClientGetResultBuilder {
  typedef ClientGetResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(ClientGetResult::VT_ACCOUNT, account, 0);
  }
  void add_amount(int64_t amount) {
    fbb_.AddElement<int64_t>(ClientGetResult::VT_AMOUNT, amount, 0);
  }
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(ClientGetResult::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  explicit ClientGetResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ClientGetResultBuilder &operator=(const ClientGetResultBuilder &);
  flatbuffers::Offset<ClientGetResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ClientGetResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<ClientGetResult> CreateClientGetResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0,
    int64_t amount = 0,
    bool success = false) {
  ClientGetResultBuilder builder_(_fbb);
  builder_.add_amount(amount);
  builder_.add_account(account);
  builder_.add_success(success);
  return builder_.Finish();
}

struct ReplicaGetResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaGetResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACCOUNT = 4,
    VT_AMOUNT = 6,
    VT_LAST_APPLIED = 8,
    VT_SUCCESS = 10
  };
  int64_t account() const {
    return GetField<int64_t>(VT_ACCOUNT, 0);
  }
  int64_t amount() const {
    return GetField<int64_t>(VT_AMOUNT, 0);
  }
  int64_t last_applied() const {
    return GetField<int64_t>(VT_LAST_APPLIED, 0);
  }
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           VerifyField<int64_t>(verifier, VT_AMOUNT) &&
           VerifyField<int64_t>(verifier, VT_LAST_APPLIED) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS) &&
           verifier.EndTable();
  }
};

struct ReplicaGetResultBuilder {
  typedef ReplicaGetResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(ReplicaGetResult::VT_ACCOUNT, account, 0);
  }
  void add_amount(int64_t amount) {
    fbb_.AddElement<int64_t>(ReplicaGetResult::VT_AMOUNT, amount, 0);
  }
  void add_last_applied(int64_t last_applied) {
    fbb_.AddElement<int64_t>(ReplicaGetResult::VT_LAST_APPLIED, last_applied, 0);
  }
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(ReplicaGetResult::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  explicit ReplicaGetResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaGetResultBuilder &operator=(const ReplicaGetResultBuilder &);
  flatbuffers::Offset<ReplicaGetResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaGetResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaGetResult> CreateReplicaGetResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0,
    int64_t amount = 0,
    int64_t last_applied = 0,
    bool success = false) {
  ReplicaGetResultBuilder builder_(_fbb);
  builder_.add_last_applied(last_applied);
  builder_.add_amount(amount);
  builder_.add_account(account);
  builder_.add_success(success);
  return builder_.Finish();
}

struct PingResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PingResultBuilder Builder;
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct PingResultBuilder {
  typedef PingResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  explicit PingResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PingResultBuilder &operator=(const PingResultBuilder &);
  flatbuffers::Offset<PingResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PingResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<PingResult> CreatePingResult(
    flatbuffers::FlatBufferBuilder &_fbb) {
  PingResultBuilder builder_(_fbb);
  return builder_.Finish();
}

struct BasicResponse FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef BasicResponseBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TYPE = 4,
    VT_TICKET = 6,
    VT_RESULT_TYPE = 8,
    VT_RESULT = 10
  };
  paxos_sgx::crash::ReqType type() const {
    return static_cast<paxos_sgx::crash::ReqType>(GetField<int8_t>(VT_TYPE, 0));
  }
  int64_t ticket() const {
    return GetField<int64_t>(VT_TICKET, 0);
  }
  paxos_sgx::crash::Result result_type() const {
    return static_cast<paxos_sgx::crash::Result>(GetField<uint8_t>(VT_RESULT_TYPE, 0));
  }
  const void *result() const {
    return GetPointer<const void *>(VT_RESULT);
  }
  template<typename T> const T *result_as() const;
  const paxos_sgx::crash::ClientGetResult *result_as_ClientGetResult() const {
    return result_type() == paxos_sgx::crash::Result_ClientGetResult ? static_cast<const paxos_sgx::crash::ClientGetResult *>(result()) : nullptr;
  }
  const paxos_sgx::crash::ReplicaGetResult *result_as_ReplicaGetResult() const {
    return result_type() == paxos_sgx::crash::Result_ReplicaGetResult ? static_cast<const paxos_sgx::crash::ReplicaGetResult *>(result()) : nullptr;
  }
  const paxos_sgx::crash::PingResult *result_as_PingResult() const {
    return result_type() == paxos_sgx::crash::Result_PingResult ? static_cast<const paxos_sgx::crash::PingResult *>(result()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE) &&
           VerifyField<int64_t>(verifier, VT_TICKET) &&
           VerifyField<uint8_t>(verifier, VT_RESULT_TYPE) &&
           VerifyOffset(verifier, VT_RESULT) &&
           VerifyResult(verifier, result(), result_type()) &&
           verifier.EndTable();
  }
};

template<> inline const paxos_sgx::crash::ClientGetResult *BasicResponse::result_as<paxos_sgx::crash::ClientGetResult>() const {
  return result_as_ClientGetResult();
}

template<> inline const paxos_sgx::crash::ReplicaGetResult *BasicResponse::result_as<paxos_sgx::crash::ReplicaGetResult>() const {
  return result_as_ReplicaGetResult();
}

template<> inline const paxos_sgx::crash::PingResult *BasicResponse::result_as<paxos_sgx::crash::PingResult>() const {
  return result_as_PingResult();
}

struct BasicResponseBuilder {
  typedef BasicResponse Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(paxos_sgx::crash::ReqType type) {
    fbb_.AddElement<int8_t>(BasicResponse::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_ticket(int64_t ticket) {
    fbb_.AddElement<int64_t>(BasicResponse::VT_TICKET, ticket, 0);
  }
  void add_result_type(paxos_sgx::crash::Result result_type) {
    fbb_.AddElement<uint8_t>(BasicResponse::VT_RESULT_TYPE, static_cast<uint8_t>(result_type), 0);
  }
  void add_result(flatbuffers::Offset<void> result) {
    fbb_.AddOffset(BasicResponse::VT_RESULT, result);
  }
  explicit BasicResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  BasicResponseBuilder &operator=(const BasicResponseBuilder &);
  flatbuffers::Offset<BasicResponse> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<BasicResponse>(end);
    return o;
  }
};

inline flatbuffers::Offset<BasicResponse> CreateBasicResponse(
    flatbuffers::FlatBufferBuilder &_fbb,
    paxos_sgx::crash::ReqType type = paxos_sgx::crash::ReqType_ping,
    int64_t ticket = 0,
    paxos_sgx::crash::Result result_type = paxos_sgx::crash::Result_NONE,
    flatbuffers::Offset<void> result = 0) {
  BasicResponseBuilder builder_(_fbb);
  builder_.add_ticket(ticket);
  builder_.add_result(result);
  builder_.add_result_type(result_type);
  builder_.add_type(type);
  return builder_.Finish();
}

inline bool VerifyResult(flatbuffers::Verifier &verifier, const void *obj, Result type) {
  switch (type) {
    case Result_NONE: {
      return true;
    }
    case Result_ClientGetResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::ClientGetResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Result_ReplicaGetResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::ReplicaGetResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Result_PingResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::PingResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyResultVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyResult(
        verifier,  values->Get(i), types->GetEnum<Result>(i))) {
      return false;
    }
  }
  return true;
}

inline const paxos_sgx::crash::BasicResponse *GetBasicResponse(const void *buf) {
  return flatbuffers::GetRoot<paxos_sgx::crash::BasicResponse>(buf);
}

inline const paxos_sgx::crash::BasicResponse *GetSizePrefixedBasicResponse(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<paxos_sgx::crash::BasicResponse>(buf);
}

inline bool VerifyBasicResponseBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<paxos_sgx::crash::BasicResponse>(nullptr);
}

inline bool VerifySizePrefixedBasicResponseBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<paxos_sgx::crash::BasicResponse>(nullptr);
}

inline void FinishBasicResponseBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::crash::BasicResponse> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedBasicResponseBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::crash::BasicResponse> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace crash
}  // namespace paxos_sgx

#endif  // FLATBUFFERS_GENERATED_CRASHRESP_PAXOS_SGX_CRASH_H_
