#pragma once

#include "metadata.h"

#include <cstdint>

namespace teems {

class get_call_ctx {
   public:
    get_call_ctx(int64_t ticket, int64_t key)
        : m_metadata_ticket(ticket), m_key(key) {}

    void set_untrusted_ticket(int64_t ticket) { m_untrusted_ticket = ticket; }

    void set_metadata(Metadata&& metadata, int64_t policy_version,
                      int64_t timestamp) {
        m_metadata = metadata;
        m_timestamp = timestamp;
        m_policy_version = policy_version;
        m_metadata_ticket = -1;
    }

    Metadata const& metadata() const& { return m_metadata; }

    int64_t metadata_ticket() const { return m_metadata_ticket; }
    int64_t untrusted_ticket() const { return m_untrusted_ticket; }

    int64_t key() const { return m_key; }
    int64_t timestamp() const { return m_timestamp; }
    int64_t policy_version() const { return m_policy_version; }

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
    // bool m_metadata_set = false;

    // hint
    std::string m_name_hint;
    std::vector<uint8_t> m_value_hint;
};

class put_call_ctx {
   public:
    put_call_ctx(int64_t ticket, int64_t key, Metadata&& metadata)
        : m_untrusted_ticket(ticket), m_key(key), m_metadata(metadata) {}

    void set_metadata_ticket(int64_t ticket) {
        m_metadata_ticket = ticket;
        m_untrusted_ticket = -1;
    }

    Metadata const& metadata() const& { return m_metadata; }

    int64_t metadata_ticket() const { return m_metadata_ticket; }
    int64_t untrusted_ticket() const { return m_untrusted_ticket; }
    int64_t key() const { return m_key; }

   private:
    int64_t m_metadata_ticket = -1;
    int64_t m_untrusted_ticket = -1;
    int64_t m_key = -1;
    Metadata m_metadata;
};

int add_get_call(int64_t metadata_ticket, int64_t key);
int add_put_call(int64_t untrusted_ticket, int64_t key, Metadata&& metadata);

int rem_get_call(int64_t super_ticket);
int rem_put_call(int64_t super_ticket);

get_call_ctx* get_get_call_ctx(int64_t teems_ticket);
put_call_ctx* get_put_call_ctx(int64_t teems_ticket);

int get_call_tickets(int64_t teems_ticket, int64_t* metadata_ticket,
                     int64_t* untrusted_ticket);

}  // namespace teems
