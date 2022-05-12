#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "async.h"

namespace teems {

enum class UntrustedStoreType {
    S3,
    Redis,
    Filesystem,
    None
};

int untrusted_change_store(UntrustedStoreType type);
int untrusted_close();

bool untrusted_get(int64_t super_ticket, uint8_t call_number, std::string const& key, std::vector<uint8_t> &value);
bool untrusted_put(int64_t super_ticket, uint8_t call_number, std::string const& key, std::vector<uint8_t> const& value);

int64_t untrusted_get_async(int64_t super_ticket, uint8_t call_number, std::string const &key);
int64_t untrusted_put_async(int64_t super_ticket, uint8_t call_number, std::string const &key, std::vector<uint8_t> const &value);

poll_state poll_untrusted(int64_t ticket);

}  // namespace teems
