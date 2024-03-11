// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PBFT_PAXOS_SGX_PBFT_H_
#define FLATBUFFERS_GENERATED_PBFT_PAXOS_SGX_PBFT_H_

#include "flatbuffers/flatbuffers.h"

namespace paxos_sgx {
namespace pbft {

struct FastGetArgs;
struct FastGetArgsBuilder;

struct FastGetResult;
struct FastGetResultBuilder;

struct OperationArgs;
struct OperationArgsBuilder;

struct OperationResult;
struct OperationResultBuilder;

struct ReplicaPrePrepare;
struct ReplicaPrePrepareBuilder;

struct ReplicaPrepare;
struct ReplicaPrepareBuilder;

struct ReplicaCommit;
struct ReplicaCommitBuilder;

struct Empty;
struct EmptyBuilder;

struct Message;
struct MessageBuilder;

enum MessageType {
  MessageType_client_fast_get_req = 0,
  MessageType_client_fast_get_resp = 1,
  MessageType_client_operation_req = 2,
  MessageType_client_operation_resp = 3,
  MessageType_replica_pre_prepare = 4,
  MessageType_replica_prepare = 5,
  MessageType_replica_commit = 6,
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
    MessageType_replica_pre_prepare,
    MessageType_replica_prepare,
    MessageType_replica_commit,
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
    "replica_pre_prepare",
    "replica_prepare",
    "replica_commit",
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
  BasicMessage_ReplicaPrePrepare = 5,
  BasicMessage_ReplicaPrepare = 6,
  BasicMessage_ReplicaCommit = 7,
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
    BasicMessage_ReplicaPrePrepare,
    BasicMessage_ReplicaPrepare,
    BasicMessage_ReplicaCommit,
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
    "ReplicaPrePrepare",
    "ReplicaPrepare",
    "ReplicaCommit",
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

template<> struct BasicMessageTraits<paxos_sgx::pbft::FastGetArgs> {
  static const BasicMessage enum_value = BasicMessage_FastGetArgs;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::FastGetResult> {
  static const BasicMessage enum_value = BasicMessage_FastGetResult;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::OperationArgs> {
  static const BasicMessage enum_value = BasicMessage_OperationArgs;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::OperationResult> {
  static const BasicMessage enum_value = BasicMessage_OperationResult;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::ReplicaPrePrepare> {
  static const BasicMessage enum_value = BasicMessage_ReplicaPrePrepare;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::ReplicaPrepare> {
  static const BasicMessage enum_value = BasicMessage_ReplicaPrepare;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::ReplicaCommit> {
  static const BasicMessage enum_value = BasicMessage_ReplicaCommit;
};

template<> struct BasicMessageTraits<paxos_sgx::pbft::Empty> {
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
    VT_LAST_APPLIED = 8,
    VT_LAST_ACCEPTED = 10
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
  int64_t last_accepted() const {
    return GetField<int64_t>(VT_LAST_ACCEPTED, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_ACCOUNT) &&
           VerifyField<int64_t>(verifier, VT_AMOUNT) &&
           VerifyField<int64_t>(verifier, VT_LAST_APPLIED) &&
           VerifyField<int64_t>(verifier, VT_LAST_ACCEPTED) &&
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
  void add_last_applied(int64_t last_applied) {
    fbb_.AddElement<int64_t>(FastGetResult::VT_LAST_APPLIED, last_applied, 0);
  }
  void add_last_accepted(int64_t last_accepted) {
    fbb_.AddElement<int64_t>(FastGetResult::VT_LAST_ACCEPTED, last_accepted, 0);
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
    int64_t last_applied = 0,
    int64_t last_accepted = 0) {
  FastGetResultBuilder builder_(_fbb);
  builder_.add_last_accepted(last_accepted);
  builder_.add_last_applied(last_applied);
  builder_.add_amount(amount);
  builder_.add_account(account);
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

struct ReplicaPrePrepare FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaPrePrepareBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SLOT_NUMBER = 4,
    VT_OPERATION = 6,
    VT_DIGEST = 8
  };
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  const paxos_sgx::pbft::OperationArgs *operation() const {
    return GetPointer<const paxos_sgx::pbft::OperationArgs *>(VT_OPERATION);
  }
  const flatbuffers::Vector<uint8_t> *digest() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_DIGEST);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           VerifyOffset(verifier, VT_OPERATION) &&
           verifier.VerifyTable(operation()) &&
           VerifyOffset(verifier, VT_DIGEST) &&
           verifier.VerifyVector(digest()) &&
           verifier.EndTable();
  }
};

struct ReplicaPrePrepareBuilder {
  typedef ReplicaPrePrepare Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaPrePrepare::VT_SLOT_NUMBER, slot_number, 0);
  }
  void add_operation(flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation) {
    fbb_.AddOffset(ReplicaPrePrepare::VT_OPERATION, operation);
  }
  void add_digest(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> digest) {
    fbb_.AddOffset(ReplicaPrePrepare::VT_DIGEST, digest);
  }
  explicit ReplicaPrePrepareBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaPrePrepareBuilder &operator=(const ReplicaPrePrepareBuilder &);
  flatbuffers::Offset<ReplicaPrePrepare> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaPrePrepare>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaPrePrepare> CreateReplicaPrePrepare(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0,
    flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> digest = 0) {
  ReplicaPrePrepareBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
  builder_.add_digest(digest);
  builder_.add_operation(operation);
  return builder_.Finish();
}

inline flatbuffers::Offset<ReplicaPrePrepare> CreateReplicaPrePrepareDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0,
    flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation = 0,
    const std::vector<uint8_t> *digest = nullptr) {
  auto digest__ = digest ? _fbb.CreateVector<uint8_t>(*digest) : 0;
  return paxos_sgx::pbft::CreateReplicaPrePrepare(
      _fbb,
      slot_number,
      operation,
      digest__);
}

struct ReplicaPrepare FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaPrepareBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SLOT_NUMBER = 4,
    VT_OPERATION = 6
  };
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  const paxos_sgx::pbft::OperationArgs *operation() const {
    return GetPointer<const paxos_sgx::pbft::OperationArgs *>(VT_OPERATION);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           VerifyOffset(verifier, VT_OPERATION) &&
           verifier.VerifyTable(operation()) &&
           verifier.EndTable();
  }
};

struct ReplicaPrepareBuilder {
  typedef ReplicaPrepare Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaPrepare::VT_SLOT_NUMBER, slot_number, 0);
  }
  void add_operation(flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation) {
    fbb_.AddOffset(ReplicaPrepare::VT_OPERATION, operation);
  }
  explicit ReplicaPrepareBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaPrepareBuilder &operator=(const ReplicaPrepareBuilder &);
  flatbuffers::Offset<ReplicaPrepare> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaPrepare>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaPrepare> CreateReplicaPrepare(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0,
    flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation = 0) {
  ReplicaPrepareBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
  builder_.add_operation(operation);
  return builder_.Finish();
}

struct ReplicaCommit FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ReplicaCommitBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SLOT_NUMBER = 4,
    VT_OPERATION = 6
  };
  int64_t slot_number() const {
    return GetField<int64_t>(VT_SLOT_NUMBER, 0);
  }
  const paxos_sgx::pbft::OperationArgs *operation() const {
    return GetPointer<const paxos_sgx::pbft::OperationArgs *>(VT_OPERATION);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SLOT_NUMBER) &&
           VerifyOffset(verifier, VT_OPERATION) &&
           verifier.VerifyTable(operation()) &&
           verifier.EndTable();
  }
};

struct ReplicaCommitBuilder {
  typedef ReplicaCommit Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_slot_number(int64_t slot_number) {
    fbb_.AddElement<int64_t>(ReplicaCommit::VT_SLOT_NUMBER, slot_number, 0);
  }
  void add_operation(flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation) {
    fbb_.AddOffset(ReplicaCommit::VT_OPERATION, operation);
  }
  explicit ReplicaCommitBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ReplicaCommitBuilder &operator=(const ReplicaCommitBuilder &);
  flatbuffers::Offset<ReplicaCommit> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ReplicaCommit>(end);
    return o;
  }
};

inline flatbuffers::Offset<ReplicaCommit> CreateReplicaCommit(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t slot_number = 0,
    flatbuffers::Offset<paxos_sgx::pbft::OperationArgs> operation = 0) {
  ReplicaCommitBuilder builder_(_fbb);
  builder_.add_slot_number(slot_number);
  builder_.add_operation(operation);
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
  paxos_sgx::pbft::MessageType type() const {
    return static_cast<paxos_sgx::pbft::MessageType>(GetField<int8_t>(VT_TYPE, 0));
  }
  int64_t ticket() const {
    return GetField<int64_t>(VT_TICKET, 0);
  }
  paxos_sgx::pbft::BasicMessage message_type() const {
    return static_cast<paxos_sgx::pbft::BasicMessage>(GetField<uint8_t>(VT_MESSAGE_TYPE, 0));
  }
  const void *message() const {
    return GetPointer<const void *>(VT_MESSAGE);
  }
  template<typename T> const T *message_as() const;
  const paxos_sgx::pbft::FastGetArgs *message_as_FastGetArgs() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_FastGetArgs ? static_cast<const paxos_sgx::pbft::FastGetArgs *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::FastGetResult *message_as_FastGetResult() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_FastGetResult ? static_cast<const paxos_sgx::pbft::FastGetResult *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::OperationArgs *message_as_OperationArgs() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_OperationArgs ? static_cast<const paxos_sgx::pbft::OperationArgs *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::OperationResult *message_as_OperationResult() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_OperationResult ? static_cast<const paxos_sgx::pbft::OperationResult *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::ReplicaPrePrepare *message_as_ReplicaPrePrepare() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_ReplicaPrePrepare ? static_cast<const paxos_sgx::pbft::ReplicaPrePrepare *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::ReplicaPrepare *message_as_ReplicaPrepare() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_ReplicaPrepare ? static_cast<const paxos_sgx::pbft::ReplicaPrepare *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::ReplicaCommit *message_as_ReplicaCommit() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_ReplicaCommit ? static_cast<const paxos_sgx::pbft::ReplicaCommit *>(message()) : nullptr;
  }
  const paxos_sgx::pbft::Empty *message_as_Empty() const {
    return message_type() == paxos_sgx::pbft::BasicMessage_Empty ? static_cast<const paxos_sgx::pbft::Empty *>(message()) : nullptr;
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

template<> inline const paxos_sgx::pbft::FastGetArgs *Message::message_as<paxos_sgx::pbft::FastGetArgs>() const {
  return message_as_FastGetArgs();
}

template<> inline const paxos_sgx::pbft::FastGetResult *Message::message_as<paxos_sgx::pbft::FastGetResult>() const {
  return message_as_FastGetResult();
}

template<> inline const paxos_sgx::pbft::OperationArgs *Message::message_as<paxos_sgx::pbft::OperationArgs>() const {
  return message_as_OperationArgs();
}

template<> inline const paxos_sgx::pbft::OperationResult *Message::message_as<paxos_sgx::pbft::OperationResult>() const {
  return message_as_OperationResult();
}

template<> inline const paxos_sgx::pbft::ReplicaPrePrepare *Message::message_as<paxos_sgx::pbft::ReplicaPrePrepare>() const {
  return message_as_ReplicaPrePrepare();
}

template<> inline const paxos_sgx::pbft::ReplicaPrepare *Message::message_as<paxos_sgx::pbft::ReplicaPrepare>() const {
  return message_as_ReplicaPrepare();
}

template<> inline const paxos_sgx::pbft::ReplicaCommit *Message::message_as<paxos_sgx::pbft::ReplicaCommit>() const {
  return message_as_ReplicaCommit();
}

template<> inline const paxos_sgx::pbft::Empty *Message::message_as<paxos_sgx::pbft::Empty>() const {
  return message_as_Empty();
}

struct MessageBuilder {
  typedef Message Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(paxos_sgx::pbft::MessageType type) {
    fbb_.AddElement<int8_t>(Message::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_ticket(int64_t ticket) {
    fbb_.AddElement<int64_t>(Message::VT_TICKET, ticket, 0);
  }
  void add_message_type(paxos_sgx::pbft::BasicMessage message_type) {
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
    paxos_sgx::pbft::MessageType type = paxos_sgx::pbft::MessageType_client_fast_get_req,
    int64_t ticket = 0,
    paxos_sgx::pbft::BasicMessage message_type = paxos_sgx::pbft::BasicMessage_NONE,
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
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::FastGetArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_FastGetResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::FastGetResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_OperationArgs: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::OperationArgs *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_OperationResult: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::OperationResult *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaPrePrepare: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::ReplicaPrePrepare *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaPrepare: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::ReplicaPrepare *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_ReplicaCommit: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::ReplicaCommit *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case BasicMessage_Empty: {
      auto ptr = reinterpret_cast<const paxos_sgx::pbft::Empty *>(obj);
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

inline const paxos_sgx::pbft::Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<paxos_sgx::pbft::Message>(buf);
}

inline const paxos_sgx::pbft::Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<paxos_sgx::pbft::Message>(buf);
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<paxos_sgx::pbft::Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<paxos_sgx::pbft::Message>(nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::pbft::Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<paxos_sgx::pbft::Message> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace pbft
}  // namespace paxos_sgx

#endif  // FLATBUFFERS_GENERATED_PBFT_PAXOS_SGX_PBFT_H_