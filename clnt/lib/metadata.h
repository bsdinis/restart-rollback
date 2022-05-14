#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <flatbuffers/flatbuffers.h>

#include "async.h"
#include "crypto.h"
#include "teems_config.h"

namespace teems {

constexpr size_t UNSTRUSTED_NAME_LEN =
    REGISTER_SIZE - KEY_LEN - IV_LEN - MAC_LEN;

std::array<uint8_t, UNSTRUSTED_NAME_LEN> gen_ustor_name();

class Metadata {
   public:
    Metadata()
        : m_key(gen_key()), m_iv(gen_iv()), m_ustor_name(gen_ustor_name()) {
        m_ustor_name[UNSTRUSTED_NAME_LEN - 1] = 0;
        m_mac.fill(0);
    }
    Metadata(
        flatbuffers::Array<uint8_t, REGISTER_SIZE> const *serialized_metadata);

    void serialize_to_flatbuffers(
        flatbuffers::Array<uint8_t, REGISTER_SIZE> *array) const;

    // encrypt a value
    // populates the hash in the metadata
    //
    bool encrypt_value(std::vector<uint8_t> const &plaintext_value,
                       std::vector<uint8_t> &encrypted_value);

    // decrypts a value
    // checks that the hash matches
    //
    bool decrypt_value(std::vector<uint8_t> const &encrypted_value,
                       std::vector<uint8_t> &decrypt_value) const;

    inline bool operator==(Metadata const &rhs) {
        return m_key == rhs.m_key && m_iv == rhs.m_iv && m_mac == rhs.m_mac &&
               m_ustor_name == rhs.m_ustor_name;
    }

    inline std::string ustor_name() const {
        return std::string((char const *)m_ustor_name.data());
    }

   private:
    std::array<uint8_t, KEY_LEN> m_key;
    std::array<uint8_t, IV_LEN> m_iv;
    std::array<uint8_t, MAC_LEN> m_mac;
    std::array<uint8_t, UNSTRUSTED_NAME_LEN> m_ustor_name;
};

int metadata_init(char const *config = "../server/default.conf",
                  char const *cert_path = "certs/client.crt",
                  char const *key_path = "certs/client.key");
int metadata_close(bool close_remote = false);

bool metadata_get(int64_t super_ticket, uint8_t call_number, bool independent,
                  int64_t key, Metadata *value, int64_t &timestamp);
bool metadata_put(int64_t super_ticket, uint8_t call_number, bool independent,
                  int64_t key, Metadata const &value, int64_t &timestamp);

int64_t metadata_get_async(int64_t super_ticket, uint8_t call_number,
                           bool independent, int64_t key);
int64_t metadata_put_async(int64_t super_ticket, uint8_t call_number,
                           bool independent, int64_t key,
                           Metadata const &value);

int metadata_get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                         Metadata &&value, int64_t timestamp);
int metadata_put_handler(size_t peer_idx, int64_t ticket, bool success,
                         int64_t timestamp);

poll_state poll_metadata(int64_t ticket);
}  // namespace teems
