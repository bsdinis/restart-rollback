#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <flatbuffers/flatbuffers.h>

#include "teems_config.h"

namespace teems {

class Metadata {
    public:
    Metadata();
    Metadata(int todo /* TODO */);
    Metadata(flatbuffers::Array<uint8_t, REGISTER_SIZE> const * serialized_metadata);

    void serialize_to_flatbuffers(flatbuffers::Array<uint8_t, REGISTER_SIZE> *array) const;

    // encrypt a value
    // populates the hash in the metadata
    //
    bool encrypt_value(std::vector<uint8_t> const& plaintext_value, std::vector<uint8_t> &encrypted_value);

    // decrypts a value
    // checks that the hash matches
    //
    bool decrypt_value(std::vector<uint8_t> const& encrypted_value, std::vector<uint8_t> &decrypt_value) const;

    private:
    // TODO
};

bool metadata_get(int64_t key, Metadata *value, int64_t &timestamp);
bool metadata_put(int64_t key, Metadata const& value, int64_t &timestamp);

int64_t metadata_get_async(int64_t key);
int64_t metadata_put_async(int64_t key, Metadata const &value);

int metadata_get_handler(size_t peer_idx, int64_t ticket, int64_t key,
                         Metadata &&value,
                         int64_t timestamp);
int metadata_put_handler(size_t peer_idx, int64_t ticket, bool success,
                         int64_t timestamp);
}  // namespace teems
