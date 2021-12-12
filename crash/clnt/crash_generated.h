// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_
#define FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_

#include "flatbuffers/flatbuffers.h"

namespace paxos_sgx {
namespace crash {

struct FastGetArgs;
struct FastGetArgsBuilder;

struct FastGetResult;
struct FastGetResultBuilder;

struct OperationArgs;
struct OperationArgsBuilder;

struct OperationResult;
struct OperationResultBuilder;

struct ReplicaPropose;
struct ReplicaProposeBuilder;

struct ReplicaAccept;
struct ReplicaAcceptBuilder;

struct ReplicaReject;
struct ReplicaRejectBuilder;

struct Empty;
struct EmptyBuilder;

struct Message;
struct MessageBuilder;

enum MessageType {
  MessageType_client_fast_get_req = 0,
  MessageType_client_fast_get_resp = 1,
  MessageType_client_operation_req = 2,
  MessageType_client_operation_resp = 3,
  MessageType_replica_propose = 4,
  MessageType_replica_accept = 5,
  MessageType_replica_reject = 6,
  MessageType_ping_req = 7,
  MessageType_ping_resp = 8,
  MessageType_reset_req = 9,
  MessageType_reset_resp = 10,
  MessageType_close_req = 11,
  MessageType_MIN = MessageType_client_fast_get_req,
  MessageType_MAX = MessageType_close_req
};

inline const MessageType (&EnumValuesMessageType())[12] {
  static const MessageType values[] = {
    MessageType_client_fast_get_req,
    MessageType_client_fast_get_resp,
    MessageType_client_operation_req,
    MessageType_client_operation_resp,
    MessageType_replica_propose,
    MessageType_replica_accept,
    MessageType_replica_reject,
    MessageType_ping_req,
    MessageType_ping_resp,
    MessageType_reset_req,
    MessageType_reset_resp,
    MessageType_close_req
  };
  return values;
}

inline const char * const *EnumNamesMessageType() {
  static const char * const names[13] = {
    "client_fast_get_req",
    "client_fast_get_resp",
    "client_operation_req",
    "client_operation_resp",
    "replica_propose",
    "replica_accept",
    "replica_reject",
    "ping_req",
    "ping_resp",
    "reset_req",
    "reset_resp",
    "close_req",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessageType(MessageType e) {
  if (flatbuffers::IsOutRange(e, MessageType_client_fast_get_req, MessageType_close_req)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessageType()[index];
}

enum BasicMessage {
  BasicMessage_NONE = 0,
  BasicMessage_FastGetArgs = 1,
  BasicMessage_FastGetResult = 2,
  BasicMessage_OperationArgs = 3,
  BasicMessage_OperationResult = 4,
  BasicMessage_ReplicaPropose = 5,
  BasicMessage_ReplicaAccept = 6,
  BasicMessage_ReplicaReject = 7,
  BasicMessage_Empty = 8,
  BasicMessage_MIN = BasicMessage_NONE,
  BasicMessage_MAX = BasicMessage_Empty
};

inline const BasicMessage (&EnumValuesBasicMessage())[9] {
  static const BasicMessage values[] = {
    BasicMessage_NONE,
    BasicMessage_FastGetArgs,
    BasicMessage_FastGetResult,
    BasicMessage_OperationArgs,
    BasicMessage_OperationResult,
    BasicMessage_ReplicaPropose,
    BasicMessage_ReplicaAccept,
    BasicMessage_ReplicaReject,
    BasicMessage_Empty
  };
  return values;
}

inline const char * const *EnumNamesBasicMessage() {
  static const char * const names[10] = {
    "NONE",
    "FastGetArgs",
    "FastGetResult",
    "OperationArgs",
    "OperationResult",
    "ReplicaPropose",
    "ReplicaAccept",
    "ReplicaReject",
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

template<> struct BasicMessageTraits<paxos_sgx::crash::FastGetArgs> {
  static const BasicMessage enum_value = BasicMessage_FastGetArgs;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::FastGetResult> {
  static const BasicMessage enum_value = BasicMessage_FastGetResult;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::OperationArgs> {
  static const BasicMessage enum_value = BasicMessage_OperationArgs;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::OperationResult> {
  static const BasicMessage enum_value = BasicMessage_OperationResult;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::ReplicaPropose> {
  static const BasicMessage enum_value = BasicMessage_ReplicaPropose;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::ReplicaAccept> {
  static const BasicMessage enum_value = BasicMessage_ReplicaAccept;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::ReplicaReject> {
  static const BasicMessage enum_value = BasicMessage_ReplicaReject;
};

template<> struct BasicMessageTraits<paxos_sgx::crash::Empty> {
  static const BasicMessage enum_value = BasicMessage_Empty;
};

bool VerifyBasicMessage(flatbuffers::Verifier &verifier, const void *obj, BasicMessage type);
bool VerifyBasicMessageVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct FastGetArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FastGetArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACCOUNT = 4
  };
  int64_t account() const {
    return GetField<int64_t>(VT_ACCOUNT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           verifier.EndTable();
  }
};

struct FastGetArgsBuilder {
  typedef FastGetArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(FastGetArgs::VT_ACCOUNT, account, 0);
  }
  explicit FastGetArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  FastGetArgsBuilder &operator=(const FastGetArgsBuilder &);
  flatbuffers::Offset<FastGetArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FastGetArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<FastGetArgs> CreateFastGetArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0) {
  FastGetArgsBuilder builder_(_fbb);
  builder_.add_account(account);
  return builder_.Finish();
}

struct FastGetResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FastGetResultBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACCOUNT = 4,
    VT_AMOUNT = 6,
    VT_SUCCESS = 8,
    VT_LAST_APPLIED = 10
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
  int64_t last_applied() const {
    return GetField<int64_t>(VT_LAST_APPLIED, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           VerifyField<int64_t>(verifier, VT_AMOUNT) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS) &&
           VerifyField<int64_t>(verifier, VT_LAST_APPLIED) &&
           verifier.EndTable();
  }
};

struct FastGetResultBuilder {
  typedef FastGetResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(FastGetResult::VT_ACCOUNT, account, 0);
  }
  void add_amount(int64_t amount) {
    fbb_.AddElement<int64_t>(FastGetResult::VT_AMOUNT, amount, 0);
  }
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(FastGetResult::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  void add_last_applied(int64_t last_applied) {
    fbb_.AddElement<int64_t>(FastGetResult::VT_LAST_APPLIED, last_applied, 0);
  }
  explicit FastGetResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  FastGetResultBuilder &operator=(const FastGetResultBuilder &);
  flatbuffers::Offset<FastGetResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FastGetResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<FastGetResult> CreateFastGetResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0,
    int64_t amount = 0,
    bool success = false,
    int64_t last_applied = 0) {
  FastGetResultBuilder builder_(_fbb);
  builder_.add_last_applied(last_applied);
  builder_.add_amount(amount);
  builder_.add_account(account);
  builder_.add_success(success);
  return builder_.Finish();
}

struct OperationArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef OperationArgsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACCOUNT = 4,
    VT_TO = 6,
    VT_AMOUNT = 8
  };
  int64_t account() const {
    return GetField<int64_t>(VT_ACCOUNT, 0);
  }
  int64_t to() const {
    return GetField<int64_t>(VT_TO, 0);
  }
  int64_t amount() const {
    return GetField<int64_t>(VT_AMOUNT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           VerifyField<int64_t>(verifier, VT_TO) &&
           VerifyField<int64_t>(verifier, VT_AMOUNT) &&
           verifier.EndTable();
  }
};

struct OperationArgsBuilder {
  typedef OperationArgs Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(OperationArgs::VT_ACCOUNT, account, 0);
  }
  void add_to(int64_t to) {
    fbb_.AddElement<int64_t>(OperationArgs::VT_TO, to, 0);
  }
  void add_amount(int64_t amount) {
    fbb_.AddElement<int64_t>(OperationArgs::VT_AMOUNT, amount, 0);
  }
  explicit OperationArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  OperationArgsBuilder &operator=(const OperationArgsBuilder &);
  flatbuffers::Offset<OperationArgs> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<OperationArgs>(end);
    return o;
  }
};

inline flatbuffers::Offset<OperationArgs> CreateOperationArgs(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0,
    int64_t to = 0,
    int64_t amount = 0) {
  OperationArgsBuilder builder_(_fbb);
  builder_.add_amount(amount);
  builder_.add_to(to);
  builder_.add_account(account);
  return builder_.Finish();
}

struct OperationResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef OperationResultBuilder Builder;
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

struct OperationResultBuilder {
  typedef OperationResult Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(int64_t account) {
    fbb_.AddElement<int64_t>(OperationResult::VT_ACCOUNT, account, 0);
  }
  void add_amount(int64_t amount) {
    fbb_.AddElement<int64_t>(OperationResult::VT_AMOUNT, amount, 0);
  }
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(OperationResult::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  explicit OperationResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  OperationResultBuilder &operator=(const OperationResultBuilder &);
  flatbuffers::Offset<OperationResult> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<OperationResult>(end);
    return o;
  }
};

inline flatbuffers::Offset<OperationResult> CreateOperationResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t account = 0,
    int64_t amount = 0,
    bool success = false) {
  OperationResultBuilder builder_(_fbb);
  builder_.add_amount(amount);
  builder_.add_account(account);
  builder_.add_success(success);
  return builder_.Finish();
}

struct ReplicaPropose FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaProposeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_OPERATION_ARGS = 4,
    VT_SLOT_NUMBER = 6
  };
  const paxos_sgx::crash::OperationArgs *operation_args() const {
    return GetPointer<const paxos_sgx::crash::OperationArgs *>(VT_OPERATION_ARGS);
  }
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_OPERATION_ARGS) &&
           verifier.VerifyTable(operation_args()) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           verifier.EndTable();
  }
};

struct ReplicaProposeBuilder {
  typedef ReplicaPropose Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_operation_args(flatbuffers::Offset<paxos_sgx::crash::OperationArgs> operation_args) {
    fbb_.AddOffset(ReplicaPropose::VT_OPERATION_ARGS, operation_args);
  }
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaPropose::VT_SLOT_NUMBER, slot_number, 0);
  }
  explicit ReplicaProposeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaProposeBuilder &operator=(const ReplicaProposeBuilder &);
  flatbuffers::Offset<ReplicaPropose> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaPropose>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaPropose> CreateReplicaPropose(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<paxos_sgx::crash::OperationArgs> operation_args = 0,
    int64_t slot_number = 0) {
  ReplicaProposeBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
  builder_.add_operation_args(operation_args);
  return builder_.Finish();
}

struct ReplicaAccept FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaAcceptBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SLOT_NUMBER = 4
  };
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           verifier.EndTable();
  }
};

struct ReplicaAcceptBuilder {
  typedef ReplicaAccept Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaAccept::VT_SLOT_NUMBER, slot_number, 0);
  }
  explicit ReplicaAcceptBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaAcceptBuilder &operator=(const ReplicaAcceptBuilder &);
  flatbuffers::Offset<ReplicaAccept> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaAccept>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaAccept> CreateReplicaAccept(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0) {
  ReplicaAcceptBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
  return builder_.Finish();
}

struct ReplicaReject FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaRejectBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SLOT_NUMBER = 4
  };
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           verifier.EndTable();
  }
};

struct ReplicaRejectBuilder {
  typedef ReplicaReject Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaReject::VT_SLOT_NUMBER, slot_number, 0);
  }
  explicit ReplicaRejectBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaRejectBuilder &operator=(const ReplicaRejectBuilder &);
  flatbuffers::Offset<ReplicaReject> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaReject>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaReject> CreateReplicaReject(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0) {
  ReplicaRejectBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
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
  EmptyBuilder &operator=(const EmptyBuilder &);
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
  paxos_sgx::crash::MessageType type() const {
    return static_cast<paxos_sgx::crash::MessageType>(GetField<int8_t>(VT_TYPE, 0));
  }
  int64_t ticket() const {
    return GetField<int64_t>(VT_TICKET, 0);
  }
  paxos_sgx::crash::BasicMessage message_type() const {
    return static_cast<paxos_sgx::crash::BasicMessage>(GetField<uint8_t>(VT_MESSAGE_TYPE, 0));
  }
  const void *message() const {
    return GetPointer<const void *>(VT_MESSAGE);
  }
  template<typename T> const T *message_as() const;
  const paxos_sgx::crash::FastGetArgs *message_as_FastGetArgs() const {
    return message_type() == paxos_sgx::crash::BasicMessage_FastGetArgs ? static_cast<const paxos_sgx::crash::FastGetArgs *>(message()) : nullptr;
  }
  const paxos_sgx::crash::FastGetResult *message_as_FastGetResult() const {
    return message_type() == paxos_sgx::crash::BasicMessage_FastGetResult ? static_cast<const paxos_sgx::crash::FastGetResult *>(message()) : nullptr;
  }
  const paxos_sgx::crash::OperationArgs *message_as_OperationArgs() const {
    return message_type() == paxos_sgx::crash::BasicMessage_OperationArgs ? static_cast<const paxos_sgx::crash::OperationArgs *>(message()) : nullptr;
  }
  const paxos_sgx::crash::OperationResult *message_as_OperationResult() const {
    return message_type() == paxos_sgx::crash::BasicMessage_OperationResult ? static_cast<const paxos_sgx::crash::OperationResult *>(message()) : nullptr;
  }
  const paxos_sgx::crash::ReplicaPropose *message_as_ReplicaPropose() const {
    return message_type() == paxos_sgx::crash::BasicMessage_ReplicaPropose ? static_cast<const paxos_sgx::crash::ReplicaPropose *>(message()) : nullptr;
  }
  const paxos_sgx::crash::ReplicaAccept *message_as_ReplicaAccept() const {
    return message_type() == paxos_sgx::crash::BasicMessage_ReplicaAccept ? static_cast<const paxos_sgx::crash::ReplicaAccept *>(message()) : nullptr;
  }
  const paxos_sgx::crash::ReplicaReject *message_as_ReplicaReject() const {
    return message_type() == paxos_sgx::crash::BasicMessage_ReplicaReject ? static_cast<const paxos_sgx::crash::ReplicaReject *>(message()) : nullptr;
  }
  const paxos_sgx::crash::Empty *message_as_Empty() const {
    return message_type() == paxos_sgx::crash::BasicMessage_Empty ? static_cast<const paxos_sgx::crash::Empty *>(message()) : nullptr;
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

template<> inline const paxos_sgx::crash::FastGetArgs *Message::message_as<paxos_sgx::crash::FastGetArgs>() const {
  return message_as_FastGetArgs();
}

template<> inline const paxos_sgx::crash::FastGetResult *Message::message_as<paxos_sgx::crash::FastGetResult>() const {
  return message_as_FastGetResult();
}

template<> inline const paxos_sgx::crash::OperationArgs *Message::message_as<paxos_sgx::crash::OperationArgs>() const {
  return message_as_OperationArgs();
}

template<> inline const paxos_sgx::crash::OperationResult *Message::message_as<paxos_sgx::crash::OperationResult>() const {
  return message_as_OperationResult();
}

template<> inline const paxos_sgx::crash::ReplicaPropose *Message::message_as<paxos_sgx::crash::ReplicaPropose>() const {
  return message_as_ReplicaPropose();
}

template<> inline const paxos_sgx::crash::ReplicaAccept *Message::message_as<paxos_sgx::crash::ReplicaAccept>() const {
  return message_as_ReplicaAccept();
}

template<> inline const paxos_sgx::crash::ReplicaReject *Message::message_as<paxos_sgx::crash::ReplicaReject>() const {
  return message_as_ReplicaReject();
}

template<> inline const paxos_sgx::crash::Empty *Message::message_as<paxos_sgx::crash::Empty>() const {
  return message_as_Empty();
}

struct MessageBuilder {
  typedef Message Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(paxos_sgx::crash::MessageType type) {
    fbb_.AddElement<int8_t>(Message::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_ticket(int64_t ticket) {
    fbb_.AddElement<int64_t>(Message::VT_TICKET, ticket, 0);
  }
  void add_message_type(paxos_sgx::crash::BasicMessage message_type) {
    fbb_.AddElement<uint8_t>(Message::VT_MESSAGE_TYPE, static_cast<uint8_t>(message_type), 0);
  }
  void add_message(flatbuffers::Offset<void> message) {
    fbb_.AddOffset(Message::VT_MESSAGE, message);
  }
  explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  MessageBuilder &operator=(const MessageBuilder &);
  flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline flatbuffers::Offset<Message> CreateMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    paxos_sgx::crash::MessageType type = paxos_sgx::crash::MessageType_client_fast_get_req,
    int64_t ticket = 0,
    paxos_sgx::crash::BasicMessage message_type = paxos_sgx::crash::BasicMessage_NONE,
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
    case BasicMessage_FastGetArgs: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::FastGetArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_FastGetResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::FastGetResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_OperationArgs: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::OperationArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_OperationResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::OperationResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaPropose: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::ReplicaPropose *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaAccept: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::ReplicaAccept *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaReject: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::ReplicaReject *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_Empty: {
      auto ptr = reinterpret_cast<const paxos_sgx::crash::Empty *>(obj);
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

inline const paxos_sgx::crash::Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<paxos_sgx::crash::Message>(buf);
}

inline const paxos_sgx::crash::Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<paxos_sgx::crash::Message>(buf);
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<paxos_sgx::crash::Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<paxos_sgx::crash::Message>(nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::crash::Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::crash::Message> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace crash
}  // namespace paxos_sgx

#endif  // FLATBUFFERS_GENERATED_CRASH_PAXOS_SGX_CRASH_H_
