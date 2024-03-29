// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RESTARTROLLBACK_REGISTER_SGX_RESTART_ROLLBACK_H_
#define FLATBUFFERS_GENERATED_RESTARTROLLBACK_REGISTER_SGX_RESTART_ROLLBACK_H_

#include "flatbuffers/flatbuffers.h"

namespace register_sgx {
namespace restart_rollback {

struct Value;

struct GetArgs;
struct GetArgsBuilder;

struct GetResult;
struct GetResultBuilder;

struct GetTimestampArgs;
struct GetTimestampArgsBuilder;

struct GetTimestampResult;
struct GetTimestampResultBuilder;

struct ProxyPutArgs;
struct ProxyPutArgsBuilder;

struct PutArgs;
struct PutArgsBuilder;

struct PutResult;
struct PutResultBuilder;

struct StabilizeArgs;
struct StabilizeArgsBuilder;

struct Greeting;
struct GreetingBuilder;

struct Empty;
struct EmptyBuilder;

struct Message;
struct MessageBuilder;

enum MessageType : int8_t {
  MessageType_client_greeting = 0,
  MessageType_proxy_get_req = 1,
  MessageType_proxy_get_resp = 2,
  MessageType_proxy_put_req = 3,
  MessageType_proxy_put_resp = 4,
  MessageType_get_req = 5,
  MessageType_get_resp = 6,
  MessageType_get_timestamp_req = 7,
  MessageType_get_timestamp_resp = 8,
  MessageType_put_req = 9,
  MessageType_put_resp = 10,
  MessageType_ping_req = 11,
  MessageType_ping_resp = 12,
  MessageType_reset_req = 13,
  MessageType_reset_resp = 14,
  MessageType_stabilize_req = 15,
  MessageType_close_req = 16,
  MessageType_MIN = MessageType_client_greeting,
  MessageType_MAX = MessageType_close_req
};

inline const MessageType (&EnumValuesMessageType())[17] {
  static const MessageType values[] = {
    MessageType_client_greeting,
    MessageType_proxy_get_req,
    MessageType_proxy_get_resp,
    MessageType_proxy_put_req,
    MessageType_proxy_put_resp,
    MessageType_get_req,
    MessageType_get_resp,
    MessageType_get_timestamp_req,
    MessageType_get_timestamp_resp,
    MessageType_put_req,
    MessageType_put_resp,
    MessageType_ping_req,
    MessageType_ping_resp,
    MessageType_reset_req,
    MessageType_reset_resp,
    MessageType_stabilize_req,
    MessageType_close_req
  };
  return values;
}

inline const char * const *EnumNamesMessageType() {
  static const char * const names[18] = {
    "client_greeting",
    "proxy_get_req",
    "proxy_get_resp",
    "proxy_put_req",
    "proxy_put_resp",
    "get_req",
    "get_resp",
    "get_timestamp_req",
    "get_timestamp_resp",
    "put_req",
    "put_resp",
    "ping_req",
    "ping_resp",
    "reset_req",
    "reset_resp",
    "stabilize_req",
    "close_req",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessageType(MessageType e) {
  if (flatbuffers::IsOutRange(e, MessageType_client_greeting, MessageType_close_req)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessageType()[index];
}

enum BasicMessage : uint8_t {
  BasicMessage_NONE = 0,
  BasicMessage_Greeting = 1,
  BasicMessage_GetArgs = 2,
  BasicMessage_GetResult = 3,
  BasicMessage_GetTimestampArgs = 4,
  BasicMessage_GetTimestampResult = 5,
  BasicMessage_ProxyPutArgs = 6,
  BasicMessage_PutArgs = 7,
  BasicMessage_PutResult = 8,
  BasicMessage_StabilizeArgs = 9,
  BasicMessage_Empty = 10,
  BasicMessage_MIN = BasicMessage_NONE,
  BasicMessage_MAX = BasicMessage_Empty
};

inline const BasicMessage (&EnumValuesBasicMessage())[11] {
  static const BasicMessage values[] = {
    BasicMessage_NONE,
    BasicMessage_Greeting,
    BasicMessage_GetArgs,
    BasicMessage_GetResult,
    BasicMessage_GetTimestampArgs,
    BasicMessage_GetTimestampResult,
    BasicMessage_ProxyPutArgs,
    BasicMessage_PutArgs,
    BasicMessage_PutResult,
    BasicMessage_StabilizeArgs,
    BasicMessage_Empty
  };
  return values;
}

inline const char * const *EnumNamesBasicMessage() {
  static const char * const names[12] = {
    "NONE",
    "Greeting",
    "GetArgs",
    "GetResult",
    "GetTimestampArgs",
    "GetTimestampResult",
    "ProxyPutArgs",
    "PutArgs",
    "PutResult",
    "StabilizeArgs",
    "Empty",
    nullptr
  };
  return names;
}

inline const char *EnumNameBasicMessage(BasicMessage e) {
  if (flatbuffers::IsOutRange(e, BasicMessage_NONE, BasicMessage_Empty)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesBasicMessage()[index];
}

template<typename T> struct BasicMessageTraits {
  static const BasicMessage enum_value = BasicMessage_NONE;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::Greeting> {
  static const BasicMessage enum_value = BasicMessage_Greeting;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::GetArgs> {
  static const BasicMessage enum_value = BasicMessage_GetArgs;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::GetResult> {
  static const BasicMessage enum_value = BasicMessage_GetResult;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::GetTimestampArgs> {
  static const BasicMessage enum_value = BasicMessage_GetTimestampArgs;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::GetTimestampResult> {
  static const BasicMessage enum_value = BasicMessage_GetTimestampResult;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::ProxyPutArgs> {
  static const BasicMessage enum_value = BasicMessage_ProxyPutArgs;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::PutArgs> {
  static const BasicMessage enum_value = BasicMessage_PutArgs;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::PutResult> {
  static const BasicMessage enum_value = BasicMessage_PutResult;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::StabilizeArgs> {
  static const BasicMessage enum_value = BasicMessage_StabilizeArgs;
};

template<> struct BasicMessageTraits<register_sgx::restart_rollback::Empty> {
  static const BasicMessage enum_value = BasicMessage_Empty;
};

bool VerifyBasicMessage(flatbuffers::Verifier &verifier, const void *obj, BasicMessage type);
bool VerifyBasicMessageVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) Value FLATBUFFERS_FINAL_CLASS {
 private:
  uint8_t data_[8];

 public:
  Value()
      : data_() {
  }
  Value(flatbuffers::span<const uint8_t, 8> _data) {
    flatbuffers::CastToArray(data_).CopyFromSpan(_data);
  }
  const flatbuffers::Array<uint8_t, 8> *data() const {
    return &flatbuffers::CastToArray(data_);
  }
  flatbuffers::Array<uint8_t, 8> *mutable_data() {
    return &flatbuffers::CastToArray(data_);
  }
};
FLATBUFFERS_STRUCT_END(Value, 8);

struct GetArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef GetArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           verifier.EndTable();
  }
};

struct GetArgsBuilder {
  typedef GetArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(GetArgs::VT_KEY, key, 0);
  }
  explicit GetArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<GetArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<GetArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<GetArgs> CreateGetArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0) {
  GetArgsBuilder builder_(_fbb);
  builder_.add_key(key);
  return builder_.Finish();
}

struct GetResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef GetResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_VALUE = 6,
    VT_TIMESTAMP = 8,
    VT_STABLE = 10,
    VT_SUSPICIOUS = 12
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  const register_sgx::restart_rollback::Value *value() const {
    return GetStruct<const register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  register_sgx::restart_rollback::Value *mutable_value() {
    return GetStruct<register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(int64_t _timestamp) {
    return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  bool stable() const {
    return GetField<uint8_t>(VT_STABLE, 0) != 0;
  }
  bool mutate_stable(bool _stable) {
    return SetField<uint8_t>(VT_STABLE, static_cast<uint8_t>(_stable), 0);
  }
  bool suspicious() const {
    return GetField<uint8_t>(VT_SUSPICIOUS, 0) != 0;
  }
  bool mutate_suspicious(bool _suspicious) {
    return SetField<uint8_t>(VT_SUSPICIOUS, static_cast<uint8_t>(_suspicious), 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           VerifyField<register_sgx::restart_rollback::Value>(verifier, VT_VALUE) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
           VerifyField<uint8_t>(verifier, VT_STABLE) &&
           VerifyField<uint8_t>(verifier, VT_SUSPICIOUS) &&
           verifier.EndTable();
  }
};

struct GetResultBuilder {
  typedef GetResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(GetResult::VT_KEY, key, 0);
  }
  void add_value(const register_sgx::restart_rollback::Value *value) {
    fbb_.AddStruct(GetResult::VT_VALUE, value);
  }
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(GetResult::VT_TIMESTAMP, timestamp, 0);
  }
  void add_stable(bool stable) {
    fbb_.AddElement<uint8_t>(GetResult::VT_STABLE, static_cast<uint8_t>(stable), 0);
  }
  void add_suspicious(bool suspicious) {
    fbb_.AddElement<uint8_t>(GetResult::VT_SUSPICIOUS, static_cast<uint8_t>(suspicious), 0);
  }
  explicit GetResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<GetResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<GetResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<GetResult> CreateGetResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0,
    const register_sgx::restart_rollback::Value *value = 0,
    int64_t timestamp = 0,
    bool stable = false,
    bool suspicious = false) {
  GetResultBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_key(key);
  builder_.add_value(value);
  builder_.add_suspicious(suspicious);
  builder_.add_stable(stable);
  return builder_.Finish();
}

struct GetTimestampArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef GetTimestampArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           verifier.EndTable();
  }
};

struct GetTimestampArgsBuilder {
  typedef GetTimestampArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(GetTimestampArgs::VT_KEY, key, 0);
  }
  explicit GetTimestampArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<GetTimestampArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<GetTimestampArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<GetTimestampArgs> CreateGetTimestampArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0) {
  GetTimestampArgsBuilder builder_(_fbb);
  builder_.add_key(key);
  return builder_.Finish();
}

struct GetTimestampResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef GetTimestampResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_TIMESTAMP = 6,
    VT_SUSPICIOUS = 8
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(int64_t _timestamp) {
    return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  bool suspicious() const {
    return GetField<uint8_t>(VT_SUSPICIOUS, 0) != 0;
  }
  bool mutate_suspicious(bool _suspicious) {
    return SetField<uint8_t>(VT_SUSPICIOUS, static_cast<uint8_t>(_suspicious), 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
           VerifyField<uint8_t>(verifier, VT_SUSPICIOUS) &&
           verifier.EndTable();
  }
};

struct GetTimestampResultBuilder {
  typedef GetTimestampResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(GetTimestampResult::VT_KEY, key, 0);
  }
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(GetTimestampResult::VT_TIMESTAMP, timestamp, 0);
  }
  void add_suspicious(bool suspicious) {
    fbb_.AddElement<uint8_t>(GetTimestampResult::VT_SUSPICIOUS, static_cast<uint8_t>(suspicious), 0);
  }
  explicit GetTimestampResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<GetTimestampResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<GetTimestampResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<GetTimestampResult> CreateGetTimestampResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0,
    int64_t timestamp = 0,
    bool suspicious = false) {
  GetTimestampResultBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_key(key);
  builder_.add_suspicious(suspicious);
  return builder_.Finish();
}

struct ProxyPutArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ProxyPutArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_VALUE = 6,
    VT_CLIENT_ID = 8
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  const register_sgx::restart_rollback::Value *value() const {
    return GetStruct<const register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  register_sgx::restart_rollback::Value *mutable_value() {
    return GetStruct<register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  int32_t client_id() const {
    return GetField<int32_t>(VT_CLIENT_ID, 0);
  }
  bool mutate_client_id(int32_t _client_id) {
    return SetField<int32_t>(VT_CLIENT_ID, _client_id, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           VerifyField<register_sgx::restart_rollback::Value>(verifier, VT_VALUE) &&
           VerifyField<int32_t>(verifier, VT_CLIENT_ID) &&
           verifier.EndTable();
  }
};

struct ProxyPutArgsBuilder {
  typedef ProxyPutArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(ProxyPutArgs::VT_KEY, key, 0);
  }
  void add_value(const register_sgx::restart_rollback::Value *value) {
    fbb_.AddStruct(ProxyPutArgs::VT_VALUE, value);
  }
  void add_client_id(int32_t client_id) {
    fbb_.AddElement<int32_t>(ProxyPutArgs::VT_CLIENT_ID, client_id, 0);
  }
  explicit ProxyPutArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ProxyPutArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ProxyPutArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<ProxyPutArgs> CreateProxyPutArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0,
    const register_sgx::restart_rollback::Value *value = 0,
    int32_t client_id = 0) {
  ProxyPutArgsBuilder builder_(_fbb);
  builder_.add_key(key);
  builder_.add_client_id(client_id);
  builder_.add_value(value);
  return builder_.Finish();
}

struct PutArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PutArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_VALUE = 6,
    VT_TIMESTAMP = 8
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  const register_sgx::restart_rollback::Value *value() const {
    return GetStruct<const register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  register_sgx::restart_rollback::Value *mutable_value() {
    return GetStruct<register_sgx::restart_rollback::Value *>(VT_VALUE);
  }
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(int64_t _timestamp) {
    return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           VerifyField<register_sgx::restart_rollback::Value>(verifier, VT_VALUE) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
           verifier.EndTable();
  }
};

struct PutArgsBuilder {
  typedef PutArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(PutArgs::VT_KEY, key, 0);
  }
  void add_value(const register_sgx::restart_rollback::Value *value) {
    fbb_.AddStruct(PutArgs::VT_VALUE, value);
  }
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(PutArgs::VT_TIMESTAMP, timestamp, 0);
  }
  explicit PutArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<PutArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PutArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<PutArgs> CreatePutArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0,
    const register_sgx::restart_rollback::Value *value = 0,
    int64_t timestamp = 0) {
  PutArgsBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_key(key);
  builder_.add_value(value);
  return builder_.Finish();
}

struct PutResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PutResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SUCCESS = 4,
    VT_TIMESTAMP = 6
  };
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  bool mutate_success(bool _success) {
    return SetField<uint8_t>(VT_SUCCESS, static_cast<uint8_t>(_success), 0);
  }
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(int64_t _timestamp) {
    return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
           verifier.EndTable();
  }
};

struct PutResultBuilder {
  typedef PutResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(PutResult::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(PutResult::VT_TIMESTAMP, timestamp, 0);
  }
  explicit PutResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<PutResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PutResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<PutResult> CreatePutResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool success = false,
    int64_t timestamp = 0) {
  PutResultBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_success(success);
  return builder_.Finish();
}

struct StabilizeArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef StabilizeArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_TIMESTAMP = 6
  };
  int64_t key() const {
    return GetField<int64_t>(VT_KEY, 0);
  }
  bool mutate_key(int64_t _key) {
    return SetField<int64_t>(VT_KEY, _key, 0);
  }
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(int64_t _timestamp) {
    return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_KEY) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
           verifier.EndTable();
  }
};

struct StabilizeArgsBuilder {
  typedef StabilizeArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_key(int64_t key) {
    fbb_.AddElement<int64_t>(StabilizeArgs::VT_KEY, key, 0);
  }
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(StabilizeArgs::VT_TIMESTAMP, timestamp, 0);
  }
  explicit StabilizeArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<StabilizeArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<StabilizeArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<StabilizeArgs> CreateStabilizeArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t key = 0,
    int64_t timestamp = 0) {
  StabilizeArgsBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_key(key);
  return builder_.Finish();
}

struct Greeting FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef GreetingBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4
  };
  int32_t id() const {
    return GetField<int32_t>(VT_ID, 0);
  }
  bool mutate_id(int32_t _id) {
    return SetField<int32_t>(VT_ID, _id, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_ID) &&
           verifier.EndTable();
  }
};

struct GreetingBuilder {
  typedef Greeting Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(int32_t id) {
    fbb_.AddElement<int32_t>(Greeting::VT_ID, id, 0);
  }
  explicit GreetingBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Greeting> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Greeting>(end);
    return o;
  }
};

inline flatbuffers::Offset<Greeting> CreateGreeting(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t id = 0) {
  GreetingBuilder builder_(_fbb);
  builder_.add_id(id);
  return builder_.Finish();
}

struct Empty FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef EmptyBuilder Builder;
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct EmptyBuilder {
  typedef Empty Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  explicit EmptyBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Empty> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Empty>(end);
    return o;
  }
};

inline flatbuffers::Offset<Empty> CreateEmpty(
    flatbuffers::FlatBufferBuilder &_fbb) {
  EmptyBuilder builder_(_fbb);
  return builder_.Finish();
}

struct Message FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MessageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TYPE = 4,
    VT_TICKET = 6,
    VT_MESSAGE_TYPE = 8,
    VT_MESSAGE = 10
  };
  register_sgx::restart_rollback::MessageType type() const {
    return static_cast<register_sgx::restart_rollback::MessageType>(GetField<int8_t>(VT_TYPE, 0));
  }
  bool mutate_type(register_sgx::restart_rollback::MessageType _type) {
    return SetField<int8_t>(VT_TYPE, static_cast<int8_t>(_type), 0);
  }
  int64_t ticket() const {
    return GetField<int64_t>(VT_TICKET, 0);
  }
  bool mutate_ticket(int64_t _ticket) {
    return SetField<int64_t>(VT_TICKET, _ticket, 0);
  }
  register_sgx::restart_rollback::BasicMessage message_type() const {
    return static_cast<register_sgx::restart_rollback::BasicMessage>(GetField<uint8_t>(VT_MESSAGE_TYPE, 0));
  }
  const void *message() const {
    return GetPointer<const void *>(VT_MESSAGE);
  }
  template<typename T> const T *message_as() const;
  const register_sgx::restart_rollback::Greeting *message_as_Greeting() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_Greeting ? static_cast<const register_sgx::restart_rollback::Greeting *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::GetArgs *message_as_GetArgs() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_GetArgs ? static_cast<const register_sgx::restart_rollback::GetArgs *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::GetResult *message_as_GetResult() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_GetResult ? static_cast<const register_sgx::restart_rollback::GetResult *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::GetTimestampArgs *message_as_GetTimestampArgs() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_GetTimestampArgs ? static_cast<const register_sgx::restart_rollback::GetTimestampArgs *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::GetTimestampResult *message_as_GetTimestampResult() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_GetTimestampResult ? static_cast<const register_sgx::restart_rollback::GetTimestampResult *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::ProxyPutArgs *message_as_ProxyPutArgs() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_ProxyPutArgs ? static_cast<const register_sgx::restart_rollback::ProxyPutArgs *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::PutArgs *message_as_PutArgs() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_PutArgs ? static_cast<const register_sgx::restart_rollback::PutArgs *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::PutResult *message_as_PutResult() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_PutResult ? static_cast<const register_sgx::restart_rollback::PutResult *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::StabilizeArgs *message_as_StabilizeArgs() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_StabilizeArgs ? static_cast<const register_sgx::restart_rollback::StabilizeArgs *>(message()) : nullptr;
  }
  const register_sgx::restart_rollback::Empty *message_as_Empty() const {
    return message_type() == register_sgx::restart_rollback::BasicMessage_Empty ? static_cast<const register_sgx::restart_rollback::Empty *>(message()) : nullptr;
  }
  void *mutable_message() {
    return GetPointer<void *>(VT_MESSAGE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE) &&
           VerifyField<int64_t>(verifier, VT_TICKET) &&
           VerifyField<uint8_t>(verifier, VT_MESSAGE_TYPE) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           VerifyBasicMessage(verifier, message(), message_type()) &&
           verifier.EndTable();
  }
};

template<> inline const register_sgx::restart_rollback::Greeting *Message::message_as<register_sgx::restart_rollback::Greeting>() const {
  return message_as_Greeting();
}

template<> inline const register_sgx::restart_rollback::GetArgs *Message::message_as<register_sgx::restart_rollback::GetArgs>() const {
  return message_as_GetArgs();
}

template<> inline const register_sgx::restart_rollback::GetResult *Message::message_as<register_sgx::restart_rollback::GetResult>() const {
  return message_as_GetResult();
}

template<> inline const register_sgx::restart_rollback::GetTimestampArgs *Message::message_as<register_sgx::restart_rollback::GetTimestampArgs>() const {
  return message_as_GetTimestampArgs();
}

template<> inline const register_sgx::restart_rollback::GetTimestampResult *Message::message_as<register_sgx::restart_rollback::GetTimestampResult>() const {
  return message_as_GetTimestampResult();
}

template<> inline const register_sgx::restart_rollback::ProxyPutArgs *Message::message_as<register_sgx::restart_rollback::ProxyPutArgs>() const {
  return message_as_ProxyPutArgs();
}

template<> inline const register_sgx::restart_rollback::PutArgs *Message::message_as<register_sgx::restart_rollback::PutArgs>() const {
  return message_as_PutArgs();
}

template<> inline const register_sgx::restart_rollback::PutResult *Message::message_as<register_sgx::restart_rollback::PutResult>() const {
  return message_as_PutResult();
}

template<> inline const register_sgx::restart_rollback::StabilizeArgs *Message::message_as<register_sgx::restart_rollback::StabilizeArgs>() const {
  return message_as_StabilizeArgs();
}

template<> inline const register_sgx::restart_rollback::Empty *Message::message_as<register_sgx::restart_rollback::Empty>() const {
  return message_as_Empty();
}

struct MessageBuilder {
  typedef Message Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(register_sgx::restart_rollback::MessageType type) {
    fbb_.AddElement<int8_t>(Message::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_ticket(int64_t ticket) {
    fbb_.AddElement<int64_t>(Message::VT_TICKET, ticket, 0);
  }
  void add_message_type(register_sgx::restart_rollback::BasicMessage message_type) {
    fbb_.AddElement<uint8_t>(Message::VT_MESSAGE_TYPE, static_cast<uint8_t>(message_type), 0);
  }
  void add_message(flatbuffers::Offset<void> message) {
    fbb_.AddOffset(Message::VT_MESSAGE, message);
  }
  explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline flatbuffers::Offset<Message> CreateMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    register_sgx::restart_rollback::MessageType type = register_sgx::restart_rollback::MessageType_client_greeting,
    int64_t ticket = 0,
    register_sgx::restart_rollback::BasicMessage message_type = register_sgx::restart_rollback::BasicMessage_NONE,
    flatbuffers::Offset<void> message = 0) {
  MessageBuilder builder_(_fbb);
  builder_.add_ticket(ticket);
  builder_.add_message(message);
  builder_.add_message_type(message_type);
  builder_.add_type(type);
  return builder_.Finish();
}

inline bool VerifyBasicMessage(flatbuffers::Verifier &verifier, const void *obj, BasicMessage type) {
  switch (type) {
    case BasicMessage_NONE: {
      return true;
    }
    case BasicMessage_Greeting: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::Greeting *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_GetArgs: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::GetArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_GetResult: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::GetResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_GetTimestampArgs: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::GetTimestampArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_GetTimestampResult: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::GetTimestampResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ProxyPutArgs: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::ProxyPutArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_PutArgs: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::PutArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_PutResult: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::PutResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_StabilizeArgs: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::StabilizeArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_Empty: {
      auto ptr = reinterpret_cast<const register_sgx::restart_rollback::Empty *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyBasicMessageVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyBasicMessage(
        verifier,  values->Get(i), types->GetEnum<BasicMessage>(i))) {
      return false;
    }
  }
  return true;
}

inline const register_sgx::restart_rollback::Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<register_sgx::restart_rollback::Message>(buf);
}

inline const register_sgx::restart_rollback::Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<register_sgx::restart_rollback::Message>(buf);
}

inline Message *GetMutableMessage(void *buf) {
  return flatbuffers::GetMutableRoot<Message>(buf);
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<register_sgx::restart_rollback::Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<register_sgx::restart_rollback::Message>(nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<register_sgx::restart_rollback::Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<register_sgx::restart_rollback::Message> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace restart_rollback
}  // namespace register_sgx

#endif  // FLATBUFFERS_GENERATED_RESTARTROLLBACK_REGISTER_SGX_RESTART_ROLLBACK_H_
