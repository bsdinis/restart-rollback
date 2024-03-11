#pragma once

#include "log.h"
#include "metadata.h"

#include <cstdint>

namespace teems {

class get_call_ctx {
   public:
    get_call_ctx(int64_t ticket, int64_t key, std::string name_hint,
                 std::vector<uint8_t>* value_hint)
        : m_metadata_ticket(ticket),
          m_key(key),
          m_name_hint(name_hint),
          m_value_set(value_hint != nullptr) {
        if (value_hint != nullptr) {
            m_value = *value_hint;
        }
    }

    void set_untrusted_ticket(int64_t ticket) { m_untrusted_ticket = ticket; }

    void set_metadata(Metadata&& metadata, int64_t policy_version,
                      int64_t timestamp) {
        m_metadata = metadata;
        m_timestamp = timestamp;
        m_policy_version = policy_version;
        m_metadata_ticket = -1;
        m_metadata_set = true;
    }

    Metadata const& metadata() const& {
        if (!m_metadata_set) {
            ERROR(
                "returning metadata from a call which has not received "
                "anything from the metadata subsystem");
        }
        return m_metadata;
    }
    bool metadata_set() const { return m_metadata_set; }

    std::vector<uint8_t> const& value() const {
        if (!m_value_set) {
            ERROR("returning a value when none is set");
        }
        return m_value;
    }
    bool value_set() const { return m_value_set; }

    void set_encrypted_value(std::vector<uint8_t>&& encrypted_value) {
        m_encrypted_value = encrypted_value;
        m_encrypted_value_set = true;
    }
    std::vector<uint8_t> const& encrypted_value() const {
        if (!m_encrypted_value_set) {
            ERROR("returning a encrypted_value when none is set");
        }
        return m_encrypted_value;
    }
    bool encrypted_value_set() const { return m_encrypted_value_set; }

    int64_t metadata_ticket() const { return m_metadata_ticket; }
    int64_t untrusted_ticket() const { return m_untrusted_ticket; }

    int64_t key() const { return m_key; }
    int64_t timestamp() const { return m_timestamp; }
    int64_t policy_version() const { return m_policy_version; }

    std::string name_hint() const { return m_name_hint; }

   private:
    // tickets
    int64_t m_metadata_ticket = -1;
    int64_t m_untrusted_ticket = -1;

    // return info
    int64_t m_key = -1;
    int64_t m_timestamp = -1;
    int64_t m_policy_version = -1;

    // metadata
    Metadata m_metadata;
    bool m_metadata_set = false;

    // encrypted value
    std::vector<uint8_t> m_encrypted_value;
    bool m_encrypted_value_set = false;

    // hint
    std::string m_name_hint;
    bool m_value_set = false;
    std::vector<uint8_t> m_value;
};

class put_call_ctx {
   public:
    put_call_ctx(int64_t ticket, int64_t key, Metadata&& metadata,
                 std::vector<uint8_t> const& value)
        : m_untrusted_ticket(ticket),
          m_key(key),
          m_metadata(metadata),
          m_value(value) {}

    void set_metadata_ticket(int64_t ticket) {
        m_metadata_ticket = ticket;
        m_untrusted_ticket = -1;
    }

    Metadata const& metadata() const& { return m_metadata; }
    std::vector<uint8_t> const& value() const& { return m_value; }

    int64_t metadata_ticket() const { return m_metadata_ticket; }
    int64_t untrusted_ticket() const { return m_untrusted_ticket; }
    int64_t key() const { return m_key; }

   private:
    int64_t m_metadata_ticket = -1;
    int64_t m_untrusted_ticket = -1;
    int64_t m_key = -1;
    Metadata m_metadata;
    std::vector<uint8_t> m_value;
};

int add_get_call(int64_t metadata_ticket, int64_t key, std::string name_hint,
                 std::vector<uint8_t>* value_hint);
int add_put_call(int64_t untrusted_ticket, int64_t key, Metadata&& metadata,
                 std::vector<uint8_t> const& value);

int rem_get_call(int64_t super_ticket);
int rem_put_call(int64_t super_ticket);

get_call_ctx* get_get_call_ctx(int64_t teems_ticket);
put_call_ctx* get_put_call_ctx(int64_t teems_ticket);

int get_call_tickets(int64_t teems_ticket, int64_t* metadata_ticket,
                     int64_t* untrusted_ticket);

}  // namespace teems
