/**
 * result for the different queries
 *
 * this is a small class hierarchy
 * derive (finally) from the base class to add different results (if required)
 */

#pragma once

#include "log.h"

namespace register_sgx {
namespace crash {

enum class result_type {
    None,
    OneVal
};

class result {
    public:
    virtual result_type type() const { return result_type::None; }
    virtual ~result() = default;
};

template<typename T>
class one_val_result final : public result {
    public:
    one_val_result() = delete;
    explicit one_val_result(T val) : val_(val) {}

    virtual result_type type() const override { return result_type::OneVal; }
    inline T get() const { return val_; }
    virtual ~one_val_result() = default;

    private:
    T val_;
};

} // namespace crash
} // namespace register_sgx
