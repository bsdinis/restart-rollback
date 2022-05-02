// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_CRASH_REGISTER_SGX_CRASH_H_
#define FLATBUFFERS_GENERATED_CRASH_REGISTER_SGX_CRASH_H_

#include "flatbuffers/flatbuffers.h"

namespace register_sgx {
namespace crash {

struct Value;

struct GetArgs;
struct GetArgsBuilder;

struct GetResult;
struct GetResultBuilder;

struct GetTimestampArgs;
struct GetTimestampArgsBuilder;

struct GetTimestampResult;
struct GetTimestampResultBuilder;

struct PutArgs;
struct PutArgsBuilder;

struct PutResult;
struct PutResultBuilder;

struct Empty;
struct EmptyBuilder;

struct Message;
struct MessageBuilder;

enum MessageType : int8_t {
    MessageType_get_req = 0,
    MessageType_get_resp = 1,
    MessageType_get_timestamp_req = 2,
    MessageType_get_timestamp_resp = 3,
    MessageType_put_req = 4,
    MessageType_put_resp = 5,
    MessageType_ping_req = 6,
    MessageType_ping_resp = 7,
    MessageType_reset_req = 8,
    MessageType_reset_resp = 9,
    MessageType_close_req = 10,
    MessageType_MIN = MessageType_get_req,
    MessageType_MAX = MessageType_close_req
};

inline const MessageType (&EnumValuesMessageType())[11] {
    static const MessageType values[] = {
        MessageType_get_req,           MessageType_get_resp,
        MessageType_get_timestamp_req, MessageType_get_timestamp_resp,
        MessageType_put_req,           MessageType_put_resp,
        MessageType_ping_req,          MessageType_ping_resp,
        MessageType_reset_req,         MessageType_reset_resp,
        MessageType_close_req};
    return values;
}

inline const char *const *EnumNamesMessageType() {
    static const char *const names[12] = {
        "get_req",   "get_resp",   "get_timestamp_req", "get_timestamp_resp",
        "put_req",   "put_resp",   "ping_req",          "ping_resp",
        "reset_req", "reset_resp", "close_req",         nullptr};
    return names;
}

inline const char *EnumNameMessageType(MessageType e) {
    if (flatbuffers::IsOutRange(e, MessageType_get_req, MessageType_close_req))
        return "";
    const size_t index = static_cast<size_t>(e);
    return EnumNamesMessageType()[index];
}

enum BasicMessage : uint8_t {
    BasicMessage_NONE = 0,
    BasicMessage_GetArgs = 1,
    BasicMessage_GetResult = 2,
    BasicMessage_GetTimestampArgs = 3,
    BasicMessage_GetTimestampResult = 4,
    BasicMessage_PutArgs = 5,
    BasicMessage_PutResult = 6,
    BasicMessage_Empty = 7,
    BasicMessage_MIN = BasicMessage_NONE,
    BasicMessage_MAX = BasicMessage_Empty
};

inline const BasicMessage (&EnumValuesBasicMessage())[8] {
    static const BasicMessage values[] = {BasicMessage_NONE,
                                          BasicMessage_GetArgs,
                                          BasicMessage_GetResult,
                                          BasicMessage_GetTimestampArgs,
                                          BasicMessage_GetTimestampResult,
                                          BasicMessage_PutArgs,
                                          BasicMessage_PutResult,
                                          BasicMessage_Empty};
    return values;
}

inline const char *const *EnumNamesBasicMessage() {
    static const char *const names[9] = {"NONE",
                                         "GetArgs",
                                         "GetResult",
                                         "GetTimestampArgs",
                                         "GetTimestampResult",
                                         "PutArgs",
                                         "PutResult",
                                         "Empty",
                                         nullptr};
    return names;
}

inline const char *EnumNameBasicMessage(BasicMessage e) {
    if (flatbuffers::IsOutRange(e, BasicMessage_NONE, BasicMessage_Empty))
        return "";
    const size_t index = static_cast<size_t>(e);
    return EnumNamesBasicMessage()[index];
}

template <typename T>
struct BasicMessageTraits {
    static const BasicMessage enum_value = BasicMessage_NONE;
};

template <>
struct BasicMessageTraits<register_sgx::crash::GetArgs> {
    static const BasicMessage enum_value = BasicMessage_GetArgs;
};

template <>
struct BasicMessageTraits<register_sgx::crash::GetResult> {
    static const BasicMessage enum_value = BasicMessage_GetResult;
};

template <>
struct BasicMessageTraits<register_sgx::crash::GetTimestampArgs> {
    static const BasicMessage enum_value = BasicMessage_GetTimestampArgs;
};

template <>
struct BasicMessageTraits<register_sgx::crash::GetTimestampResult> {
    static const BasicMessage enum_value = BasicMessage_GetTimestampResult;
};

template <>
struct BasicMessageTraits<register_sgx::crash::PutArgs> {
    static const BasicMessage enum_value = BasicMessage_PutArgs;
};

template <>
struct BasicMessageTraits<register_sgx::crash::PutResult> {
    static const BasicMessage enum_value = BasicMessage_PutResult;
};

template <>
struct BasicMessageTraits<register_sgx::crash::Empty> {
    static const BasicMessage enum_value = BasicMessage_Empty;
};

bool VerifyBasicMessage(flatbuffers::Verifier &verifier, const void *obj,
                        BasicMessage type);
bool VerifyBasicMessageVector(
    flatbuffers::Verifier &verifier,
    const flatbuffers::Vector<flatbuffers::Offset<void>> *values,
    const flatbuffers::Vector<uint8_t> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) Value FLATBUFFERS_FINAL_CLASS {
   private:
    uint8_t data_[2048];

   public:
    Value() : data_() {}
    Value(flatbuffers::span<const uint8_t, 2048> _data) {
        flatbuffers::CastToArray(data_).CopyFromSpan(_data);
    }
    const flatbuffers::Array<uint8_t, 2048> *data() const {
        return &flatbuffers::CastToArray(data_);
    }
    flatbuffers::Array<uint8_t, 2048> *mutable_data() {
        return &flatbuffers::CastToArray(data_);
    }
};
FLATBUFFERS_STRUCT_END(Value, 2048);

struct GetArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef GetArgsBuilder Builder;
    enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
        VT_KEY = 4
    };
    int64_t key() const { return GetField<int64_t>(VT_KEY, 0); }
    bool mutate_key(int64_t _key) { return SetField<int64_t>(VT_KEY, _key, 0); }
    bool Verify(flatbuffers::Verifier &verifier) const {
        return VerifyTableStart(verifier) &&
               VerifyField<int64_t>(verifier, VT_KEY) && verifier.EndTable();
    }
};

struct GetArgsBuilder {
    typedef GetArgs Table;
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_key(int64_t key) {
        fbb_.AddElement<int64_t>(GetArgs::VT_KEY, key, 0);
    }
    explicit GetArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
        start_ = fbb_.StartTable();
    }
    flatbuffers::Offset<GetArgs> Finish() {
        const auto end = fbb_.EndTable(start_);
        auto o = flatbuffers::Offset<GetArgs>(end);
        return o;
    }
};

inline flatbuffers::Offset<GetArgs> CreateGetArgs(
    flatbuffers::FlatBufferBuilder &_fbb, int64_t key = 0) {
    GetArgsBuilder builder_(_fbb);
    builder_.add_key(key);
    return builder_.Finish();
}

struct GetResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef GetResultBuilder Builder;
    enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
        VT_KEY = 4,
        VT_VALUE = 6,
        VT_TIMESTAMP = 8
    };
    int64_t key() const { return GetField<int64_t>(VT_KEY, 0); }
    bool mutate_key(int64_t _key) { return SetField<int64_t>(VT_KEY, _key, 0); }
    const register_sgx::crash::Value *value() const {
        return GetStruct<const register_sgx::crash::Value *>(VT_VALUE);
    }
    register_sgx::crash::Value *mutable_value() {
        return GetStruct<register_sgx::crash::Value *>(VT_VALUE);
    }
    int64_t timestamp() const { return GetField<int64_t>(VT_TIMESTAMP, 0); }
    bool mutate_timestamp(int64_t _timestamp) {
        return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
    }
    bool Verify(flatbuffers::Verifier &verifier) const {
        return VerifyTableStart(verifier) &&
               VerifyField<int64_t>(verifier, VT_KEY) &&
               VerifyField<register_sgx::crash::Value>(verifier, VT_VALUE) &&
               VerifyField<int64_t>(verifier, VT_TIMESTAMP) &&
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
    void add_value(const register_sgx::crash::Value *value) {
        fbb_.AddStruct(GetResult::VT_VALUE, value);
    }
    void add_timestamp(int64_t timestamp) {
        fbb_.AddElement<int64_t>(GetResult::VT_TIMESTAMP, timestamp, 0);
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
    flatbuffers::FlatBufferBuilder &_fbb, int64_t key = 0,
    const register_sgx::crash::Value *value = 0, int64_t timestamp = 0) {
    GetResultBuilder builder_(_fbb);
    builder_.add_timestamp(timestamp);
    builder_.add_key(key);
    builder_.add_value(value);
    return builder_.Finish();
}

struct GetTimestampArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef GetTimestampArgsBuilder Builder;
    enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
        VT_KEY = 4
    };
    int64_t key() const { return GetField<int64_t>(VT_KEY, 0); }
    bool mutate_key(int64_t _key) { return SetField<int64_t>(VT_KEY, _key, 0); }
    bool Verify(flatbuffers::Verifier &verifier) const {
        return VerifyTableStart(verifier) &&
               VerifyField<int64_t>(verifier, VT_KEY) && verifier.EndTable();
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
    flatbuffers::FlatBufferBuilder &_fbb, int64_t key = 0) {
    GetTimestampArgsBuilder builder_(_fbb);
    builder_.add_key(key);
    return builder_.Finish();
}

struct GetTimestampResult FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef GetTimestampResultBuilder Builder;
    enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
        VT_KEY = 4,
        VT_TIMESTAMP = 6
    };
    int64_t key() const { return GetField<int64_t>(VT_KEY, 0); }
    bool mutate_key(int64_t _key) { return SetField<int64_t>(VT_KEY, _key, 0); }
    int64_t timestamp() const { return GetField<int64_t>(VT_TIMESTAMP, 0); }
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

struct GetTimestampResultBuilder {
    typedef GetTimestampResult Table;
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_key(int64_t key) {
        fbb_.AddElement<int64_t>(GetTimestampResult::VT_KEY, key, 0);
    }
    void add_timestamp(int64_t timestamp) {
        fbb_.AddElement<int64_t>(GetTimestampResult::VT_TIMESTAMP, timestamp,
                                 0);
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
    flatbuffers::FlatBufferBuilder &_fbb, int64_t key = 0,
    int64_t timestamp = 0) {
    GetTimestampResultBuilder builder_(_fbb);
    builder_.add_timestamp(timestamp);
    builder_.add_key(key);
    return builder_.Finish();
}

struct PutArgs FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef PutArgsBuilder Builder;
    enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
        VT_KEY = 4,
        VT_VALUE = 6,
        VT_TIMESTAMP = 8
    };
    int64_t key() const { return GetField<int64_t>(VT_KEY, 0); }
    bool mutate_key(int64_t _key) { return SetField<int64_t>(VT_KEY, _key, 0); }
    const register_sgx::crash::Value *value() const {
        return GetStruct<const register_sgx::crash::Value *>(VT_VALUE);
    }
    register_sgx::crash::Value *mutable_value() {
        return GetStruct<register_sgx::crash::Value *>(VT_VALUE);
    }
    int64_t timestamp() const { return GetField<int64_t>(VT_TIMESTAMP, 0); }
    bool mutate_timestamp(int64_t _timestamp) {
        return SetField<int64_t>(VT_TIMESTAMP, _timestamp, 0);
    }
    bool Verify(flatbuffers::Verifier &verifier) const {
        return VerifyTableStart(verifier) &&
               VerifyField<int64_t>(verifier, VT_KEY) &&
               VerifyField<register_sgx::crash::Value>(verifier, VT_VALUE) &&
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
    void add_value(const register_sgx::crash::Value *value) {
        fbb_.AddStruct(PutArgs::VT_VALUE, value);
    }
    void add_timestamp(int64_t timestamp) {
        fbb_.AddElement<int64_t>(PutArgs::VT_TIMESTAMP, timestamp, 0);
    }
    explicit PutArgsBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
        start_ = fbb_.StartTable();
    }
    flatbuffers::Offset<PutArgs> Finish() {
        const auto end = fbb_.EndTable(start_);
        auto o = flatbuffers::Offset<PutArgs>(end);
        return o;
    }
};

inline flatbuffers::Offset<PutArgs> CreatePutArgs(
    flatbuffers::FlatBufferBuilder &_fbb, int64_t key = 0,
    const register_sgx::crash::Value *value = 0, int64_t timestamp = 0) {
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
    bool success() const { return GetField<uint8_t>(VT_SUCCESS, 0) != 0; }
    bool mutate_success(bool _success) {
        return SetField<uint8_t>(VT_SUCCESS, static_cast<uint8_t>(_success), 0);
    }
    int64_t timestamp() const { return GetField<int64_t>(VT_TIMESTAMP, 0); }
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
        fbb_.AddElement<uint8_t>(PutResult::VT_SUCCESS,
                                 static_cast<uint8_t>(success), 0);
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
    flatbuffers::FlatBufferBuilder &_fbb, bool success = false,
    int64_t timestamp = 0) {
    PutResultBuilder builder_(_fbb);
    builder_.add_timestamp(timestamp);
    builder_.add_success(success);
    return builder_.Finish();
}

struct Empty FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
    typedef EmptyBuilder Builder;
    bool Verify(flatbuffers::Verifier &verifier) const {
        return VerifyTableStart(verifier) && verifier.EndTable();
    }
};

struct EmptyBuilder {
    typedef Empty Table;
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    explicit EmptyBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
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
    register_sgx::crash::MessageType type() const {
        return static_cast<register_sgx::crash::MessageType>(
            GetField<int8_t>(VT_TYPE, 0));
    }
    bool mutate_type(register_sgx::crash::MessageType _type) {
        return SetField<int8_t>(VT_TYPE, static_cast<int8_t>(_type), 0);
    }
    int64_t ticket() const { return GetField<int64_t>(VT_TICKET, 0); }
    bool mutate_ticket(int64_t _ticket) {
        return SetField<int64_t>(VT_TICKET, _ticket, 0);
    }
    register_sgx::crash::BasicMessage message_type() const {
        return static_cast<register_sgx::crash::BasicMessage>(
            GetField<uint8_t>(VT_MESSAGE_TYPE, 0));
    }
    const void *message() const { return GetPointer<const void *>(VT_MESSAGE); }
    template <typename T>
    const T *message_as() const;
    const register_sgx::crash::GetArgs *message_as_GetArgs() const {
        return message_type() == register_sgx::crash::BasicMessage_GetArgs
                   ? static_cast<const register_sgx::crash::GetArgs *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::GetResult *message_as_GetResult() const {
        return message_type() == register_sgx::crash::BasicMessage_GetResult
                   ? static_cast<const register_sgx::crash::GetResult *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::GetTimestampArgs *message_as_GetTimestampArgs()
        const {
        return message_type() ==
                       register_sgx::crash::BasicMessage_GetTimestampArgs
                   ? static_cast<const register_sgx::crash::GetTimestampArgs *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::GetTimestampResult *
    message_as_GetTimestampResult() const {
        return message_type() ==
                       register_sgx::crash::BasicMessage_GetTimestampResult
                   ? static_cast<
                         const register_sgx::crash::GetTimestampResult *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::PutArgs *message_as_PutArgs() const {
        return message_type() == register_sgx::crash::BasicMessage_PutArgs
                   ? static_cast<const register_sgx::crash::PutArgs *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::PutResult *message_as_PutResult() const {
        return message_type() == register_sgx::crash::BasicMessage_PutResult
                   ? static_cast<const register_sgx::crash::PutResult *>(
                         message())
                   : nullptr;
    }
    const register_sgx::crash::Empty *message_as_Empty() const {
        return message_type() == register_sgx::crash::BasicMessage_Empty
                   ? static_cast<const register_sgx::crash::Empty *>(message())
                   : nullptr;
    }
    void *mutable_message() { return GetPointer<void *>(VT_MESSAGE); }
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

template <>
inline const register_sgx::crash::GetArgs *
Message::message_as<register_sgx::crash::GetArgs>() const {
    return message_as_GetArgs();
}

template <>
inline const register_sgx::crash::GetResult *
Message::message_as<register_sgx::crash::GetResult>() const {
    return message_as_GetResult();
}

template <>
inline const register_sgx::crash::GetTimestampArgs *
Message::message_as<register_sgx::crash::GetTimestampArgs>() const {
    return message_as_GetTimestampArgs();
}

template <>
inline const register_sgx::crash::GetTimestampResult *
Message::message_as<register_sgx::crash::GetTimestampResult>() const {
    return message_as_GetTimestampResult();
}

template <>
inline const register_sgx::crash::PutArgs *
Message::message_as<register_sgx::crash::PutArgs>() const {
    return message_as_PutArgs();
}

template <>
inline const register_sgx::crash::PutResult *
Message::message_as<register_sgx::crash::PutResult>() const {
    return message_as_PutResult();
}

template <>
inline const register_sgx::crash::Empty *
Message::message_as<register_sgx::crash::Empty>() const {
    return message_as_Empty();
}

struct MessageBuilder {
    typedef Message Table;
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_type(register_sgx::crash::MessageType type) {
        fbb_.AddElement<int8_t>(Message::VT_TYPE, static_cast<int8_t>(type), 0);
    }
    void add_ticket(int64_t ticket) {
        fbb_.AddElement<int64_t>(Message::VT_TICKET, ticket, 0);
    }
    void add_message_type(register_sgx::crash::BasicMessage message_type) {
        fbb_.AddElement<uint8_t>(Message::VT_MESSAGE_TYPE,
                                 static_cast<uint8_t>(message_type), 0);
    }
    void add_message(flatbuffers::Offset<void> message) {
        fbb_.AddOffset(Message::VT_MESSAGE, message);
    }
    explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
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
    register_sgx::crash::MessageType type =
        register_sgx::crash::MessageType_get_req,
    int64_t ticket = 0,
    register_sgx::crash::BasicMessage message_type =
        register_sgx::crash::BasicMessage_NONE,
    flatbuffers::Offset<void> message = 0) {
    MessageBuilder builder_(_fbb);
    builder_.add_ticket(ticket);
    builder_.add_message(message);
    builder_.add_message_type(message_type);
    builder_.add_type(type);
    return builder_.Finish();
}

inline bool VerifyBasicMessage(flatbuffers::Verifier &verifier, const void *obj,
                               BasicMessage type) {
    switch (type) {
        case BasicMessage_NONE: {
            return true;
        }
        case BasicMessage_GetArgs: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::GetArgs *>(obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_GetResult: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::GetResult *>(obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_GetTimestampArgs: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::GetTimestampArgs *>(
                    obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_GetTimestampResult: {
            auto ptr = reinterpret_cast<
                const register_sgx::crash::GetTimestampResult *>(obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_PutArgs: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::PutArgs *>(obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_PutResult: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::PutResult *>(obj);
            return verifier.VerifyTable(ptr);
        }
        case BasicMessage_Empty: {
            auto ptr =
                reinterpret_cast<const register_sgx::crash::Empty *>(obj);
            return verifier.VerifyTable(ptr);
        }
        default:
            return true;
    }
}

inline bool VerifyBasicMessageVector(
    flatbuffers::Verifier &verifier,
    const flatbuffers::Vector<flatbuffers::Offset<void>> *values,
    const flatbuffers::Vector<uint8_t> *types) {
    if (!values || !types) return !values && !types;
    if (values->size() != types->size()) return false;
    for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
        if (!VerifyBasicMessage(verifier, values->Get(i),
                                types->GetEnum<BasicMessage>(i))) {
            return false;
        }
    }
    return true;
}

inline const register_sgx::crash::Message *GetMessage(const void *buf) {
    return flatbuffers::GetRoot<register_sgx::crash::Message>(buf);
}

inline const register_sgx::crash::Message *GetSizePrefixedMessage(
    const void *buf) {
    return flatbuffers::GetSizePrefixedRoot<register_sgx::crash::Message>(buf);
}

inline Message *GetMutableMessage(void *buf) {
    return flatbuffers::GetMutableRoot<Message>(buf);
}

inline bool VerifyMessageBuffer(flatbuffers::Verifier &verifier) {
    return verifier.VerifyBuffer<register_sgx::crash::Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(flatbuffers::Verifier &verifier) {
    return verifier.VerifySizePrefixedBuffer<register_sgx::crash::Message>(
        nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<register_sgx::crash::Message> root) {
    fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<register_sgx::crash::Message> root) {
    fbb.FinishSizePrefixed(root);
}

}  // namespace crash
}  // namespace register_sgx

#endif  // FLATBUFFERS_GENERATED_CRASH_REGISTER_SGX_CRASH_H_
