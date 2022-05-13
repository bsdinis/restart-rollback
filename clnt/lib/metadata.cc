#include "metadata.h"

#include "config.h"
#include "log.h"
#include "network.h"
#include "peer.h"
#include "result.h"
#include "ssl_util.h"
#include "teems_generated.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <vector>

namespace teems {

//==========================================
// HELPER FUNCTIONS INTERFACE
//==========================================
namespace {
int64_t send_metadata_get_request(peer &server, int64_t super_ticket,
                                  uint8_t call_number, int64_t key,
                                  call_type type);
int64_t send_metadata_put_request(peer &server, int64_t super_ticket,
                                  uint8_t call_number, int64_t key,
                                  Metadata const &value, call_type type);

int metadata_get_handler_sync(int64_t ticket, int64_t key, Metadata &&value,
                              int64_t timestamp);
int metadata_get_handler_async(int64_t ticket, int64_t key, Metadata &&value,
                               int64_t timestamp);

int metadata_put_handler_sync(int64_t ticket, bool success, int64_t timestamp);
int metadata_put_handler_async(int64_t ticket, bool success, int64_t timestamp);

}  // anonymous namespace

//==========================================
// GLOBAL VARIABLES
//==========================================
// metadata get
std::tuple<int64_t, Metadata, int64_t> g_metadata_get_result;

// metadata put
std::pair<int64_t, bool> g_metadata_put_result;

//==========================================
// EXTERNAL VARIABLES
//==========================================

extern int32_t g_client_id;

extern ::std::unordered_map<int64_t, std::unique_ptr<result>> g_results_map;
extern ::std::vector<peer> g_servers;
extern timeval g_timeout;

extern ssize_t g_calls_issued;
extern ssize_t g_calls_concluded;

extern SSL_CTX *g_client_ctx;
extern ::std::vector<peer> g_servers;

//==========================================
// METADATA
//==========================================

std::array<uint8_t, UNSTRUSTED_NAME_LEN> gen_ustor_name() {
    static bool initialized = false;

    if (!initialized) {
        timespec current;
        clock_gettime(CLOCK_REALTIME, &current);
        srand(current.tv_sec + current.tv_nsec);
        srand48(current.tv_sec + current.tv_nsec);
        initialized = true;
    }

    std::array<uint8_t, UNSTRUSTED_NAME_LEN> name;
    for (size_t idx = 0; idx < UNSTRUSTED_NAME_LEN; ++idx) {
        uint8_t b = 0;
        do {
            b = rand() % 256;
        } while (!isalnum(b));

        name[idx] = b;
    }

    return name;
}

template <size_t N>
std::string array_to_str(std::array<uint8_t, N> const &arr) {
    char str[N * 2 + 1];
    for (ssize_t idx = 0; idx < N; ++idx) {
        str[2 * idx] = "0123456789abcdef"[arr[idx] / 16];
        str[2 * idx + 1] = "0123456789abcdef"[arr[idx] % 16];
    }

    str[2 * N] = 0;
    return std::string(str);
}

template <size_t N>
std::string fb_array_to_str(flatbuffers::Array<uint8_t, N> const *arr) {
    char str[N * 2 + 1];
    for (ssize_t idx = 0; idx < N; ++idx) {
        str[2 * idx] = "0123456789abcdef"[arr->Data()[idx] / 16];
        str[2 * idx + 1] = "0123456789abcdef"[arr->Data()[idx] % 16];
    }

    str[2 * N] = 0;
    return std::string(str);
}

Metadata::Metadata(
    flatbuffers::Array<uint8_t, REGISTER_SIZE> const *serialized_metadata) {
    std::copy(serialized_metadata->cbegin(),
              serialized_metadata->cbegin() + KEY_LEN, std::begin(m_key));
    std::copy(serialized_metadata->cbegin() + KEY_LEN,
              serialized_metadata->cbegin() + KEY_LEN + IV_LEN,
              std::begin(m_iv));
    std::copy(serialized_metadata->cbegin() + KEY_LEN + IV_LEN,
              serialized_metadata->cbegin() + KEY_LEN + IV_LEN + MAC_LEN,
              std::begin(m_mac));
    std::copy(serialized_metadata->cbegin() + KEY_LEN + IV_LEN + MAC_LEN,
              serialized_metadata->cbegin() + KEY_LEN + IV_LEN + MAC_LEN +
                  UNSTRUSTED_NAME_LEN,
              std::begin(m_ustor_name));
}

void Metadata::serialize_to_flatbuffers(
    flatbuffers::Array<uint8_t, REGISTER_SIZE> *array) const {
    std::copy(std::cbegin(m_key), std::cend(m_key), array->Data());
    std::copy(std::cbegin(m_iv), std::cend(m_iv), array->Data() + KEY_LEN);
    std::copy(std::cbegin(m_mac), std::cend(m_mac),
              array->Data() + KEY_LEN + IV_LEN);
    std::copy(std::cbegin(m_ustor_name), std::cend(m_ustor_name),
              array->Data() + KEY_LEN + IV_LEN + MAC_LEN);
}

bool Metadata::encrypt_value(std::vector<uint8_t> const &plaintext,
                             std::vector<uint8_t> &ciphertext) {
    if (aes_encrypt(plaintext, m_key, m_iv, m_mac, ciphertext) != 0) {
        ERROR("failed to encrypt value");
        return false;
    }

    return true;
}
bool Metadata::decrypt_value(std::vector<uint8_t> const &ciphertext,
                             std::vector<uint8_t> &plaintext) const {
    if (aes_decrypt(ciphertext, m_key, m_iv, m_mac, plaintext) != 0) {
        ERROR("failed to decrypt value (MAC check may have failed)");
        return false;
    }

    return true;
}
//==========================================
// LIBRARY IMPLEMENTATION
//==========================================

int metadata_init(char const *config, char const *cert_path,
                  char const *key_path) {
    config_t conf;
    if (config_parse(&conf, config) == -1) {
        ERROR("failed to stat configuration: %s", config);
        return -1;
    }

    g_servers.reserve(conf.size);

    if (init_client_ssl_ctx(&g_client_ctx) == -1) {
        ERROR("failed to setup ssl ctx");
        config_free(&conf);
        return -1;
    }

    if (load_certificates(g_client_ctx, cert_path, key_path) == -1) {
        ERROR("failed load certs");
        config_free(&conf);
        close_ssl_ctx(g_client_ctx);
        return -1;
    }

    for (ssize_t idx = 0; idx < conf.size; ++idx) {
        auto const &peer_node = conf.nodes[idx];
        if (connect_to_proxy(peer_node) == -1) {
            ERROR("failed to connect to server on %s:%d", peer_node.addr,
                  peer_node.port);
            config_free(&conf);
            close_ssl_ctx(g_client_ctx);
            return -1;
        }

        INFO("connected to server on %s:%d", peer_node.addr, peer_node.port);
    }

    config_free(&conf);

    while (g_client_id == -1) {
        struct timeval timeout = g_timeout;
        auto res = process_peers(&timeout);
        if (res == process_res::ERR) {
            return -1;
        }
    }

    return 0;
}

int metadata_close(bool close_remote) {
    // this will close the remote server
    if (close_remote) {
        int64_t const ticket = gen_teems_ticket(call_type::Sync);
        flatbuffers::FlatBufferBuilder builder;
        auto close_args = teems::CreateEmpty(builder);
        auto request =
            teems::CreateMessage(builder, teems::MessageType_close_req, ticket,
                                 teems::BasicMessage_Empty, close_args.Union());
        builder.Finish(request);

        size_t const size = builder.GetSize();
        uint8_t const *payload = builder.GetBufferPointer();

        for (size_t idx = 0; idx < g_servers.size(); ++idx) {
            auto &server = g_servers[idx];
            if (server.append(&size, 1) == -1) {
                // encode message header
                ERROR("failed to prepare message to send");
                return -1;
            }
            if (server.append(payload, size) ==
                -1) {  // then the segment itself
                ERROR("failed to prepare message to send");
                return -1;
            }

            server.flush();
            process_peer(idx, server, nullptr);  // block, gets released by EOF
        }
    }

    for (auto &server : g_servers) {
        server.close();
    }

    close_ssl_ctx(g_client_ctx);
    g_servers.clear();

    return 0;
}

// sync
bool metadata_get(int64_t super_ticket, uint8_t call_number, int64_t key,
                  Metadata *value, int64_t &timestamp) {
    if (send_metadata_get_request(g_servers[0], super_ticket, call_number, key,
                                  call_type::Sync) == -1) {
        ERROR("Failed to send get");
        return false;
    }

    if (block_until_return(0, g_servers[0]) == -1) {
        ERROR("failed to get a return from the basicserver");
        return false;
    }

    timestamp = std::get<2>(g_metadata_get_result);
    *value = std::get<1>(g_metadata_get_result);

    return true;
}

bool metadata_put(int64_t super_ticket, uint8_t call_number, int64_t key,
                  Metadata const &value, int64_t &timestamp) {
    if (send_metadata_put_request(g_servers[0], super_ticket, call_number, key,
                                  value, call_type::Sync) == -1) {
        ERROR("Failed to sent put");
        return false;
    }

    if (block_until_return(0, g_servers[0]) == -1) {
        ERROR("failed to get a return from the basicserver");
        return false;
    }

    timestamp = std::get<0>(g_metadata_put_result);
    return std::get<1>(g_metadata_put_result);
}

// async
int64_t metadata_get_async(int64_t super_ticket, uint8_t call_number,
                           int64_t key) {
    return send_metadata_get_request(g_servers[0], super_ticket, call_number,
                                     key, call_type::Async);
}
int64_t metadata_put_async(int64_t super_ticket, uint8_t call_number,
                           int64_t key, Metadata const &value) {
    return send_metadata_put_request(g_servers[0], super_ticket, call_number,
                                     key, value, call_type::Async);
}

// handlers
int metadata_get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                         Metadata &&value, int64_t timestamp) {
    g_calls_concluded += 1;
    switch (ticket_call_type(ticket)) {
        case call_type::Sync:
            return metadata_get_handler_sync(ticket, key, std::move(value),
                                             timestamp);
        case call_type::Async:
            return metadata_get_handler_async(ticket, key, std::move(value),
                                              timestamp);
        default:
            ERROR("invalid call type for sub call");
            return -1;
    }
    // unreachable (could use __builtin_unreachable)
    return -1;
}

int metadata_put_handler(size_t peer_idx, int64_t ticket, bool success,
                         int64_t timestamp) {
    g_calls_concluded += 1;
    switch (ticket_call_type(ticket)) {
        case call_type::Sync:  // Sync
            return metadata_put_handler_sync(ticket, success, timestamp);
        case call_type::Async:  // Async
            return metadata_put_handler_async(ticket, success, timestamp);
        default:
            ERROR("invalid call type for sub call");
            return -1;
    }
}

poll_state poll_metadata(int64_t ticket) {
    if (std::any_of(std::cbegin(g_servers), std::cend(g_servers),
                    [](peer const &server) { return !server.connected(); })) {
        ERROR("no connection to proxies");
        return poll_state::ERR;
    }

    timeval timeout = g_timeout;
    auto const res = process_peer(0, g_servers[0], &timeout);
    switch (res) {
        case process_res::ERR:
            ERROR("connection broke");
            return poll_state::ERR;

        case process_res::HANDLED_MSG:
            return (ticket == -1 || has_result(ticket)) ? poll_state::READY
                                                        : poll_state::PENDING;

        case process_res::NOOP:
            return poll_state::PENDING;
    }

    return n_calls_outlasting() == 0 ? poll_state::NO_CALLS
                                     : poll_state::PENDING;
}

//==========================================
// HELPER FUNCTIONS IMPLEMENTATION
//==========================================
namespace {
int64_t send_metadata_get_request(peer &server, int64_t super_ticket,
                                  uint8_t call_number, int64_t key,
                                  call_type type) {
    int64_t const ticket = gen_metadata_ticket(super_ticket, call_number, type);
    flatbuffers::FlatBufferBuilder builder;
    auto get_args = teems::CreateGetArgs(builder, key);

    auto request =
        teems::CreateMessage(builder, teems::MessageType_proxy_get_req, ticket,
                             teems::BasicMessage_GetArgs, get_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    g_calls_issued += 1;
    return ticket;
}

int64_t send_metadata_put_request(peer &server, int64_t super_ticket,
                                  uint8_t call_number, int64_t key,
                                  Metadata const &value, call_type type) {
    int64_t const ticket = gen_metadata_ticket(super_ticket, call_number, type);
    flatbuffers::FlatBufferBuilder builder;

    auto fb_value = teems::Value();
    value.serialize_to_flatbuffers(fb_value.mutable_data());

    auto put_args =
        teems::CreateProxyPutArgs(builder, key, &fb_value, g_client_id);

    auto request = teems::CreateMessage(
        builder, teems::MessageType_proxy_put_req, ticket,
        teems::BasicMessage_ProxyPutArgs, put_args.Union());
    builder.Finish(request);

    size_t const size = builder.GetSize();
    uint8_t const *payload = builder.GetBufferPointer();

    if (server.append(&size, 1) == -1) {
        // encode message header
        ERROR("failed to prepare message to send");
        return -1;
    }
    if (server.append(payload, size) == -1) {  // then the segment itself
        ERROR("failed to prepare message to send");
        return -1;
    }

    g_calls_issued += 1;
    return ticket;
}

// metadata get
int metadata_get_handler_sync(int64_t ticket, int64_t key, Metadata &&value,
                              int64_t timestamp) {
    g_metadata_get_result = std::make_tuple(key, std::move(value), timestamp);
    return 0;
}
int metadata_get_handler_async(int64_t ticket, int64_t key, Metadata &&value,
                               int64_t timestamp) {
    g_results_map.emplace(
        ticket,
        std::unique_ptr<result>(
            std::make_unique<
                one_val_result<std::tuple<int64_t, Metadata, int64_t>>>(
                one_val_result<std::tuple<int64_t, Metadata, int64_t>>(
                    std::make_tuple(key, std::move(value), timestamp)))));
    return 0;
}

// metadata put
int metadata_put_handler_sync(int64_t ticket, bool success, int64_t timestamp) {
    g_metadata_put_result = std::make_pair(timestamp, success);
    return 0;
}
int metadata_put_handler_async(int64_t ticket, bool success,
                               int64_t timestamp) {
    g_results_map.emplace(
        ticket, std::unique_ptr<result>(
                    std::make_unique<one_val_result<std::pair<int64_t, bool>>>(
                        one_val_result<std::pair<int64_t, bool>>(
                            std::make_pair(timestamp, success)))));
    return 0;
}
}  // anonymous namespace
}  // namespace teems
