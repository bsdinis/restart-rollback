#pragma once

#include "qp_config.h"
#include "byzantine_generated.h"

namespace register_sgx {
namespace byzantine {

int sign_value(int64_t key, int64_t timestamp, std::array<uint8_t, REGISTER_SIZE> const & value, std::vector<uint8_t> &signature);
int verify_value(SignedValue const * value, bool * authentic);

} // namespace byzantine
} // namespace register_sgx
