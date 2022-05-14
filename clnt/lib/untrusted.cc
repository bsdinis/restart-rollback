#include "untrusted.h"

#include "log.h"
#include "protocol_helpers.h"
#include "result.h"

#include <libs3.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <memory>
#include <unordered_map>

#include <hiredis/async.h>
#include <hiredis/hiredis.h>

#define LIBS3_NEWER_VERSION

namespace teems {
//==========================================
// EXTERNAL VARIABLES
//==========================================

extern ::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;
extern timeval g_timeout;

namespace {

//==============================
// GLOBALS
//==============================
UntrustedStoreType g_ustore_type = UntrustedStoreType::None;

//==============================
// CONFIGURATION
//==============================

// S3
constexpr const char *g_s3_access_key = "AKIAQKQSOMVLXBBU7B7B";
constexpr const char *g_s3_secret_key =
    "UmV6mcDh29H6BMIAkALh+kgJgL/CeSc2KucGPO5z";

constexpr const char *g_s3_host = "s3.eu-west-1.amazonaws.com";
constexpr const char *g_s3_sample_bucket = "prof-s3";
constexpr const char *g_s3_region = "eu-west-1";

// REDIS
int16_t const g_redis_port = 6379;
// constexpr const char *g_redis_host = "139.19.171.111";  // sigilium01
// constexpr const char * g_redis_host = "139.19.171.112"; // sigilium02
constexpr const char *g_redis_host = "127.0.0.1";  // localhost

// FILESYSTEM
// constexpr const char *G_DIR_PREFIX = "/local/bdinisa/teems_ustor";
constexpr const char *G_DIR_PREFIX = "/home/bsd/dev/teems/clnt/teems_ustor";
std::string g_fs_dirname;

void finish_put(int64_t ticket, bool success);
void finish_get(int64_t ticket, bool success, std::vector<uint8_t> &&value);

//==============================
// S3 INTERFACE
//==============================

int s3_init();
int s3_close();

bool s3_get(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> &value);
bool s3_put(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> const &value);

int64_t s3_get_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key);
int64_t s3_put_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key,
                     std::vector<uint8_t> const &value);

poll_state s3_poll(int64_t ticket);

//==============================
// REDIS INTERFACE
//==============================
redisContext *g_redis_ctx = nullptr;
redisAsyncContext *g_redis_async_ctx = nullptr;

int redis_init();
int redis_close();

bool redis_get(int64_t super_ticket, uint8_t call_number, bool independent,
               std::string const &key, std::vector<uint8_t> &value);
bool redis_put(int64_t super_ticket, uint8_t call_number, bool independent,
               std::string const &key, std::vector<uint8_t> const &value);

int64_t redis_get_async(int64_t super_ticket, uint8_t call_number,
                        bool independent, std::string const &key);
int64_t redis_put_async(int64_t super_ticket, uint8_t call_number,
                        bool independent, std::string const &key,
                        std::vector<uint8_t> const &value);

poll_state redis_poll(int64_t ticket);

//==============================
// FILESYSTEM INTERFACE
//==============================
int fs_init();
int fs_close();

bool fs_get(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> &value);
bool fs_put(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> const &value);

int64_t fs_get_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key);
int64_t fs_put_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key,
                     std::vector<uint8_t> const &value);

poll_state fs_poll(int64_t ticket);
}  // anonymous namespace

//==============================
// LIBRARY IMPLEMENTATION
//==============================

int untrusted_change_store(UntrustedStoreType type) {
    if (untrusted_close() == -1) {
        return -1;
    }
    g_ustore_type = type;
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            return 0;
        case UntrustedStoreType::S3:
            return s3_init();
        case UntrustedStoreType::Redis:
            return redis_init();
        case UntrustedStoreType::Filesystem:
            return fs_init();
    }
}

int untrusted_close() {
    UntrustedStoreType old_type = g_ustore_type;
    g_ustore_type = UntrustedStoreType::None;
    switch (old_type) {
        case UntrustedStoreType::None:
            return 0;
        case UntrustedStoreType::S3:
            return s3_close();
        case UntrustedStoreType::Redis:
            return redis_close();
        case UntrustedStoreType::Filesystem:
            return fs_close();
    }
}

bool untrusted_get(int64_t super_ticket, uint8_t call_number, bool independent,
                   std::string const &key, std::vector<uint8_t> &value) {
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            ERROR("untrusted storage is not initialized");
            return false;
        case UntrustedStoreType::S3:
            return s3_get(super_ticket, call_number, independent, key, value);
        case UntrustedStoreType::Redis:
            return redis_get(super_ticket, call_number, independent, key,
                             value);
        case UntrustedStoreType::Filesystem:
            return fs_get(super_ticket, call_number, independent, key, value);
    }
}
bool untrusted_put(int64_t super_ticket, uint8_t call_number, bool independent,
                   std::string const &key, std::vector<uint8_t> const &value) {
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            ERROR("untrusted storage is not initialized");
            return false;
        case UntrustedStoreType::S3:
            return s3_put(super_ticket, call_number, independent, key, value);
        case UntrustedStoreType::Redis:
            return redis_put(super_ticket, call_number, independent, key,
                             value);
        case UntrustedStoreType::Filesystem:
            return fs_put(super_ticket, call_number, independent, key, value);
    }
}

int64_t untrusted_get_async(int64_t super_ticket, uint8_t call_number,
                            bool independent, std::string const &key) {
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            ERROR("untrusted storage is not initialized");
            return -1;
        case UntrustedStoreType::S3:
            return s3_get_async(super_ticket, call_number, independent, key);
        case UntrustedStoreType::Redis:
            return redis_get_async(super_ticket, call_number, independent, key);
        case UntrustedStoreType::Filesystem:
            return fs_get_async(super_ticket, call_number, independent, key);
    }
}
int64_t untrusted_put_async(int64_t super_ticket, uint8_t call_number,
                            bool independent, std::string const &key,
                            std::vector<uint8_t> const &value) {
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            ERROR("untrusted storage is not initialized");
            return -1;
        case UntrustedStoreType::S3:
            return s3_put_async(super_ticket, call_number, independent, key,
                                value);
        case UntrustedStoreType::Redis:
            return redis_put_async(super_ticket, call_number, independent, key,
                                   value);
        case UntrustedStoreType::Filesystem:
            return fs_put_async(super_ticket, call_number, independent, key,
                                value);
            break;
    }
}

poll_state poll_untrusted(int64_t ticket) {
    switch (g_ustore_type) {
        case UntrustedStoreType::None:
            ERROR("untrusted storage is not initialized");
            return poll_state::Error;
        case UntrustedStoreType::S3:
            return s3_poll(ticket);
        case UntrustedStoreType::Redis:
            return redis_poll(ticket);
        case UntrustedStoreType::Filesystem:
            return fs_poll(ticket);
            break;
    }
}

namespace {
//==============================
// LIBRARY HELPERS
//==============================
void finish_put(int64_t ticket, bool success) {
    finished_call(ticket);

    if (ticket_independent(ticket)) {
        g_results_map.emplace(
            ticket,
            std::unique_ptr<result>(std::make_unique<one_val_result<bool>>(
                one_val_result<bool>(success))));
    } else {
        if (teems_handle_untrusted_put(ticket, success) != 0) {
            ERROR("failed to handle untrusted put");
        }
    }
}
void finish_get(int64_t ticket, bool success, std::vector<uint8_t> &&value) {
    finished_call(ticket);

    if (ticket_independent(ticket)) {
        g_results_map.emplace(
            ticket,
            std::unique_ptr<result>(
                std::make_unique<
                    one_val_result<std::tuple<bool, std::vector<uint8_t>>>>(
                    one_val_result<std::tuple<bool, std::vector<uint8_t>>>(
                        std::make_tuple(success, std::move(value))))));
    } else {
        if (teems_handle_untrusted_get(ticket, success, std::move(value)) !=
            0) {
            ERROR("failed to handle untrusted get");
        }
    }
}

//==============================
// S3 INTERFACE
//==============================

// state of a synchronous put request
struct s3_sync_put_ctx {
    s3_sync_put_ctx(std::vector<uint8_t> const *value_ptr)
        : m_value_ptr(value_ptr) {}

    inline size_t to_write() const { return m_value_ptr->size() - m_written; }
    inline uint8_t const *ptr() const {
        return m_value_ptr->data() + m_written;
    }

    void wrote(size_t n) { m_written += n; }
    std::vector<uint8_t> const *m_value_ptr = nullptr;
    size_t m_written = 0;
};

// state of a synchronous get request
struct s3_sync_get_ctx {
    s3_sync_get_ctx(std::vector<uint8_t> *value_ptr) : m_value_ptr(value_ptr) {
        m_value_ptr->clear();
    }

    std::vector<uint8_t> *m_value_ptr = nullptr;
    bool m_success = false;
};

// state of a synchronous put request
struct s3_async_put_ctx {
    s3_async_put_ctx(int64_t ticket, std::vector<uint8_t> const &value)
        : m_ticket(ticket), m_value(value) {}

    inline size_t to_write() const { return m_value.size() - m_written; }
    inline uint8_t const *ptr() const { return m_value.data() + m_written; }

    void wrote(size_t n) { m_written += n; }

    int64_t m_ticket = -1;
    std::vector<uint8_t> m_value;
    ssize_t m_written = 0;
};

// state of a synchronous get request
struct s3_async_get_ctx {
    s3_async_get_ctx(int64_t ticket) : m_ticket(ticket) {}

    int64_t m_ticket = -1;
    bool m_success = false;
    std::vector<uint8_t> m_value;
};

S3BucketContext const g_s3_bucket_ctx = {
    g_s3_host,      g_s3_sample_bucket, S3ProtocolHTTPS,
    S3UriStylePath, g_s3_access_key,    g_s3_secret_key,
#ifdef LIBS3_NEWER_VERSION
    nullptr,        g_s3_region
#endif
};

S3Status s3_resp_prop_cb(S3ResponseProperties const *properties,
                         void *callbackData);
void s3_sync_resp_complete_cb(S3Status status, S3ErrorDetails const *error,
                              void *callbackData);
void s3_async_put_cb(S3Status status, S3ErrorDetails const *error,
                     void *callbackData);
void s3_async_get_cb(S3Status status, S3ErrorDetails const *error,
                     void *callbackData);

S3Status s3_sync_get_object_cb(int bufferSize, const char *buffer,
                               void *callbackData);
int s3_sync_put_object_cb(int bufferSize, char *buffer, void *callbackData);
S3Status s3_async_get_object_cb(int bufferSize, const char *buffer,
                                void *callbackData);
int s3_async_put_object_cb(int bufferSize, char *buffer, void *callbackData);

S3ResponseHandler const g_s3_sync_resp_handler = {&s3_resp_prop_cb,
                                                  &s3_sync_resp_complete_cb};
S3ResponseHandler const g_s3_async_put_handler = {&s3_resp_prop_cb,
                                                  &s3_async_put_cb};
S3ResponseHandler const g_s3_async_get_handler = {&s3_resp_prop_cb,
                                                  &s3_async_get_cb};

S3RequestContext *g_s3_request_ctx = nullptr;

int s3_init() {
    if (S3_initialize(nullptr, S3_INIT_ALL, g_s3_host) != S3StatusOK) {
        ERROR("failed to init s3");
        return -1;
    }

    if (S3_create_request_context(&g_s3_request_ctx) != S3StatusOK) {
        ERROR("failed to init s3");
        return -1;
    }

    INFO("connected to S3");
    return 0;
}
int s3_close() {
    S3_destroy_request_context(g_s3_request_ctx);
    S3_deinitialize();
    return 0;
}

bool s3_get(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> &value) {
    s3_sync_get_ctx ctx(&value);
    S3GetObjectHandler get_obj_handler = {g_s3_sync_resp_handler,
                                          &s3_sync_get_object_cb};

    S3_get_object(&g_s3_bucket_ctx, key.c_str(),
                  NULL,  // no conditions
                  0,     // start at byte 0
                  0,     // end at the end
                  NULL,  // request is synchronous
#ifdef LIBS3_NEWER_VERSION
                  0,  // no timeout
#endif
                  &get_obj_handler, &ctx);

    return ctx.m_success;
}

bool s3_put(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> const &value) {
    s3_sync_put_ctx ctx(&value);
    S3PutObjectHandler put_obj_handler = {g_s3_sync_resp_handler,
                                          &s3_sync_put_object_cb};

    S3_put_object(&g_s3_bucket_ctx, key.c_str(),
                  value.size(),  // send X bytes
                  nullptr,       // put properties
                  nullptr,       // request is synchronous
#ifdef LIBS3_NEWER_VERSION
                  0,  // no timeout
#endif
                  &put_obj_handler, &ctx);

    return true;
}

int64_t s3_get_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key) {
    s3_async_get_ctx *ctx = new s3_async_get_ctx(gen_untrusted_ticket(
        super_ticket, call_number, independent, call_type::Async));

    S3GetObjectHandler get_obj_handler = {g_s3_async_get_handler,
                                          &s3_async_get_object_cb};

    S3_get_object(&g_s3_bucket_ctx, key.c_str(),
                  nullptr,           // no conditions
                  0,                 // start at byte 0
                  0,                 // end at the end
                  g_s3_request_ctx,  // request is asynchronous
#ifdef LIBS3_NEWER_VERSION
                  0,  // no timeout
#endif
                  &get_obj_handler, ctx);

    issued_call(ctx->m_ticket);
    return ctx->m_ticket;
}
int64_t s3_put_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key,
                     std::vector<uint8_t> const &value) {
    s3_async_put_ctx *ctx = new s3_async_put_ctx(
        gen_untrusted_ticket(super_ticket, call_number, independent,
                             call_type::Async),
        value);

    S3PutObjectHandler put_obj_handler = {g_s3_async_put_handler,
                                          &s3_async_put_object_cb};

    S3_put_object(&g_s3_bucket_ctx, key.c_str(),
                  value.size(),      // send X bytes
                  nullptr,           // put properties
                  g_s3_request_ctx,  // request is asynchronous
#ifdef LIBS3_NEWER_VERSION
                  0,  // no timeout
#endif
                  &put_obj_handler, ctx);

    issued_call(ctx->m_ticket);
    return ctx->m_ticket;
}

S3Status s3_resp_prop_cb(S3ResponseProperties const *properties,
                         void *callbackData) {
    return S3StatusOK;
}

void s3_sync_resp_complete_cb(S3Status status, S3ErrorDetails const *error,
                              void *callbackData) {
    return;
}

void s3_async_put_cb(S3Status status, S3ErrorDetails const *error,
                     void *callbackData) {
    if (status != S3StatusOK) {
        ERROR("S3 error: %s", error->message);
    }

    if (callbackData == nullptr) {
        ERROR("S3 put does not have callback data");
        return;
    }

    s3_async_put_ctx *ctx = (s3_async_put_ctx *)callbackData;
    int64_t ticket = ctx->m_ticket;
    delete ctx;

    finish_put(ticket, status == S3StatusOK);
    return;
}

void s3_async_get_cb(S3Status status, S3ErrorDetails const *error,
                     void *callbackData) {
    if (status != S3StatusOK) {
        LOG("S3 error: %s", error->message);
    }

    if (callbackData == NULL) {
        LOG("don't have any callback data");
        return;
    }

    s3_async_get_ctx *ctx = (s3_async_get_ctx *)callbackData;
    int64_t ticket = ctx->m_ticket;
    std::vector<uint8_t> value = std::move(ctx->m_value);
    delete ctx;

    finish_get(ticket, status == S3StatusOK, std::move(value));
    return;
}

int s3_sync_put_object_cb(int bufferSize, char *buffer, void *callbackData) {
    s3_sync_put_ctx *ctx = (s3_sync_put_ctx *)callbackData;

    int to_write = ctx->to_write();

    if (to_write > 0) {
        to_write = std::min(to_write, bufferSize);
        memcpy(buffer, ctx->ptr(), to_write);
    }

    ctx->wrote(to_write);
    return to_write;
}

S3Status s3_sync_get_object_cb(int bufferSize, const char *buffer,
                               void *callbackData) {
    s3_sync_get_ctx *ctx = (s3_sync_get_ctx *)callbackData;

    if (bufferSize <= 0 || buffer == nullptr) {
        FINE("s3 get early exit");
        return S3StatusOK;
    }

    ctx->m_success = true;
    std::copy(buffer, buffer + bufferSize,
              std::back_inserter(*ctx->m_value_ptr));
    return S3StatusOK;
}

int s3_async_put_object_cb(int bufferSize, char *buffer, void *callbackData) {
    s3_async_put_ctx *ctx = (s3_async_put_ctx *)callbackData;

    int to_write = ctx->to_write();

    if (to_write > 0) {
        to_write = std::min(to_write, bufferSize);
        memcpy(buffer, ctx->ptr(), to_write);
    }

    ctx->wrote(to_write);
    return to_write;
}

S3Status s3_async_get_object_cb(int bufferSize, const char *buffer,
                                void *callbackData) {
    s3_async_get_ctx *ctx = (s3_async_get_ctx *)callbackData;

    if (bufferSize <= 0 || buffer == nullptr) {
        FINE("s3 get early exit");
        return S3StatusOK;
    }

    ctx->m_success = true;
    std::copy(buffer, buffer + bufferSize, std::back_inserter(ctx->m_value));
    return S3StatusOK;
}

poll_state s3_poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }

    int remaining = 0;
    if (S3_runonce_request_context(g_s3_request_ctx, &remaining) !=
        S3StatusOK) {
        ERROR("failed to run S3 requests");
        return poll_state::Pending;
    }

    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }

    return poll_state::Pending;
}

//==============================
// REDIS INTERFACE
//==============================

int redis_handle_error(void) {
    if (g_redis_ctx->err == 1 && redisReconnect(g_redis_ctx) == REDIS_OK) {
        return 0;
    }

    if (g_redis_ctx->err) {
        ERROR("Redis error (%d): %s", g_redis_ctx->err, g_redis_ctx->errstr);
        return -1;
    }

    return 0;
}

int redis_init() {
    signal(SIGPIPE, SIG_IGN);

    g_redis_ctx = redisConnect(g_redis_host, g_redis_port);
    if (g_redis_ctx == nullptr || g_redis_ctx->err) {
        if (g_redis_ctx) {
            ERROR("redis error connecting: %s", g_redis_ctx->errstr);
            return -1;
        } else {
            return -1;
        }
    }

    if (redisEnableKeepAlive(g_redis_ctx) == REDIS_ERR) {
        ERROR("Redis: failed to enable keepalive");
        redisFree(g_redis_ctx);
        return -1;
    }

    g_redis_async_ctx = redisAsyncConnect(g_redis_host, g_redis_port);
    if (g_redis_async_ctx == nullptr || g_redis_async_ctx->err) {
        if (g_redis_async_ctx) {
            ERROR("redis error connecting: %s", g_redis_async_ctx->errstr);
            return -1;
        } else {
            return -1;
        }
    }

    INFO("connected to Redis server on %s:%d\n", g_redis_host, g_redis_port);
    return 0;
}
int redis_close() {
    redisFree(g_redis_ctx);
    redisAsyncFree(g_redis_async_ctx);
    return 0;
}

bool redis_get(int64_t super_ticket, uint8_t call_number, bool independent,
               std::string const &key, std::vector<uint8_t> &value) {
    redisReply *reply =
        (redisReply *)redisCommand(g_redis_ctx, "GET %s", key.c_str());
    if (reply == nullptr) {
        ERROR("redis get: null reply");
        if (redis_handle_error() == -1) {
            return false;
        }

        FINE("redis: handled error");
        reply = (redisReply *)redisCommand(g_redis_ctx, "GET %s", key.c_str());
        if (reply == nullptr) {
            ERROR("redis get failed retry");
            return false;
        }
    }

    value.clear();

    if (reply->type == REDIS_REPLY_NIL) {
        FINE("object not found: %s", key.c_str());
        freeReplyObject(reply);
        return false;
    }

    value.reserve(reply->len);
    std::copy(reply->str, reply->str + reply->len, std::back_inserter(value));
    freeReplyObject(reply);
    return true;
}

bool redis_put(int64_t super_ticket, uint8_t call_number, bool independent,
               std::string const &key, std::vector<uint8_t> const &value) {
    redisReply *reply = (redisReply *)redisCommand(
        g_redis_ctx, "SET %s %b", key.c_str(), value.data(), value.size());
    if (!reply && redis_handle_error() == -1) {
        return false;
    }

    freeReplyObject(reply);

    return true;
}

void redis_read_cb(redisAsyncContext *ac, void *reply, void *privdata) {
    int64_t *ticket_ptr = (int64_t *)privdata;
    assert(ticket_ptr != nullptr);
    int32_t const ticket = *ticket_ptr;
    free(ticket_ptr);

    std::vector<uint8_t> value;
    bool value_exists = false;
    if (reply != nullptr) {
        redisReply *redis_reply = (redisReply *)reply;
        if (redis_reply->type == REDIS_REPLY_NIL) {
            FINE("object not found: %s", key.c_str());
        } else {
            value_exists = true;
            value.reserve(redis_reply->len);
            std::copy(redis_reply->str, redis_reply->str + redis_reply->len,
                      std::back_inserter(value));
        }
    }

    finish_get(ticket, value_exists, std::move(value));
}
int64_t redis_get_async(int64_t super_ticket, uint8_t call_number,
                        bool independent, std::string const &key) {
    int64_t *ticket = (int64_t *)malloc(sizeof(int64_t));
    if (ticket == nullptr) {
        ERROR("failed to allocate ticket: %s", strerror(errno));
        return -1;
    }
    *ticket = gen_untrusted_ticket(super_ticket, call_number, independent,
                                   call_type::Async);

    if (redisAsyncCommand(g_redis_async_ctx, redis_read_cb, (void *)ticket,
                          "GET %s", key.c_str()) != REDIS_OK) {
        ERROR("failed to issue async write command");
        free(ticket);
        return -1;
    }

    issued_call(*ticket);
    return *ticket;
}

void redis_write_cb(redisAsyncContext *ac, void *reply, void *privdata) {
    int64_t *ticket_ptr = (int64_t *)privdata;
    assert(ticket_ptr != nullptr);
    int32_t const ticket = *ticket_ptr;
    free(ticket_ptr);
    finish_put(ticket, reply != nullptr);
}
int64_t redis_put_async(int64_t super_ticket, uint8_t call_number,
                        bool independent, std::string const &key,
                        std::vector<uint8_t> const &value) {
    int64_t *ticket = (int64_t *)malloc(sizeof(int64_t));
    if (ticket == nullptr) {
        ERROR("failed to allocate ticket: %s", strerror(errno));
        return -1;
    }
    *ticket = gen_untrusted_ticket(super_ticket, call_number, independent,
                                   call_type::Async);

    if (redisAsyncCommand(g_redis_async_ctx, redis_write_cb, (void *)ticket,
                          "SET %s %b", key.c_str(), value.data(),
                          value.size()) != REDIS_OK) {
        ERROR("failed to issue async write command");
        free(ticket);
        return -1;
    }

    issued_call(*ticket);
    return *ticket;
}

poll_state redis_poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }

    int const fd = g_redis_async_ctx->c.fd;
    fd_set read_fds, write_fds, except_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    FD_SET(fd, &read_fds);
    FD_SET(fd, &except_fds);
    if (sdslen(g_redis_async_ctx->c.obuf) > 0) {
        FD_SET(fd, &write_fds);
    }

    struct timeval timeout = g_timeout;
    int const activity =
        select(fd + 1, &read_fds, &write_fds, &except_fds, &timeout);
    switch (activity) {
        case -1:
            ERROR("failed to select on redis async socket (%d): %s", fd,
                  strerror(errno));
            return poll_state::Pending;
        case 0:
            return poll_state::Pending;

        default:
            if (FD_ISSET(fd, &read_fds)) {
                redisAsyncHandleRead(g_redis_async_ctx);
                if (g_redis_async_ctx->err != 0) {
                    ERROR("failed to read: %s", g_redis_async_ctx->errstr);
                }
            }
            if (FD_ISSET(fd, &write_fds)) {
                redisAsyncHandleWrite(g_redis_async_ctx);
                if (g_redis_async_ctx->err != 0) {
                    ERROR("failed to write: %s", g_redis_async_ctx->errstr);
                }
            }
            if (FD_ISSET(fd, &except_fds)) {
                ERROR("exception on redis async socket");
            }
    }

    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }

    return poll_state::Pending;
}

//==============================
// FILESYSTEM INTERFACE
//==============================

std::string fs_construct_filename(std::string const &key) {
    return g_fs_dirname + std::string("/") + key;
}

int fs_init() {
    g_fs_dirname =
        std::string(G_DIR_PREFIX) + std::string(".") + std::to_string(getpid());
    int ret = 0;
    do {
        ret = mkdir(g_fs_dirname.c_str(), 0755);
        if (ret != 0) {
            if (errno != EEXIST) {
                ERROR("failed to create dir for untrusted store @ %s: %s",
                      g_fs_dirname.c_str(), strerror(errno));
                return -1;
            }
            char cmd[100];
            snprintf(cmd, 100, "rm -r %s", g_fs_dirname.c_str());
            system(cmd);
        }
    } while (ret != 0);

    INFO("initialized local untrusted storage in %s", g_fs_dirname.c_str());
    return 0;
}

int fs_close() { return 0; }

bool fs_get(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> &value) {
    std::string const pathname = fs_construct_filename(key);
    if (access(pathname.c_str(), F_OK) != 0) {
        FINE("object does not exist: %s", pathname.c_str());
        return false;
    }

    std::ifstream stream(pathname, std::ios::in | std::ios::binary);
    value = std::vector<uint8_t>(std::istreambuf_iterator<char>(stream),
                                 std::istreambuf_iterator<char>());
    return true;
}

bool fs_put(int64_t super_ticket, uint8_t call_number, bool independent,
            std::string const &key, std::vector<uint8_t> const &value) {
    std::string const pathname = fs_construct_filename(key);
    if (access(pathname.c_str(), F_OK) == 0) {
        ERROR("object name taken: %s", pathname.c_str());
        return false;
    }

    FILE *fp = std::fopen(pathname.c_str(), "w");
    if (fp == nullptr) {
        ERROR("failed to open file for untrusted storage @ %s: %s",
              pathname.c_str(), strerror(errno));
        return false;
    }

    fwrite((void *)value.data(), sizeof(uint8_t), value.size(), fp);
    fclose(fp);
    return true;
}

int64_t fs_get_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key) {
    int64_t const ticket = gen_untrusted_ticket(super_ticket, call_number,
                                                independent, call_type::Async);
    issued_call(ticket);

    std::vector<uint8_t> value;
    bool success = fs_get(super_ticket, call_number, independent, key, value);

    finish_get(ticket, success, std::move(value));
    return ticket;
}
int64_t fs_put_async(int64_t super_ticket, uint8_t call_number,
                     bool independent, std::string const &key,
                     std::vector<uint8_t> const &value) {
    int64_t const ticket = gen_untrusted_ticket(super_ticket, call_number,
                                                independent, call_type::Async);
    issued_call(ticket);
    bool success = fs_put(super_ticket, call_number, independent, key, value);

    finish_put(ticket, success);
    return ticket;
}

poll_state fs_poll(int64_t ticket) {
    if (ticket != -1 && has_result(ticket)) {
        return poll_state::Ready;
    }

    return poll_state::NoCalls;
}

}  // namespace
}  // namespace teems
