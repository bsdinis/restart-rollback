#include "perf.h"

namespace register_sgx {
namespace restart_rollback {
namespace perf {

void* perf_stats::data() { return nullptr; }
size_t perf_stats::size() { return 0; }
// void perf_stats::print(FILE * stream) {} // this doesn't even compile

#ifndef INTRUSIVE_PERF
void perf_recorder::add(std::string const& name, double measurement) {}
void perf_recorder::add(std::string const& name) {}
#else
void perf_recorder::add(std::string const& name, double measurement) {
    int const id = name_map.at(name);
    auto res = std::find_if(this->measure_ptr, this->measure_ptr + this->size,
                            [id](measure const& m) { return m.id() == id; });
    if (res == this->measure_ptr + this->size)
        ERROR("Name %s not found in perf measures", name.c_str());
    else
        res->add(measurement);
}

void perf_recorder::add(std::string const& name) {
    int const id = name_map.at(name);
    auto res = std::find_if(this->measure_ptr, this->measure_ptr + this->size,
                            [id](measure const& m) { return m.id() == id; });
    if (res == this->measure_ptr + this->size)
        ERROR("Name %s not found in perf measures", name.c_str());
    else
        res->add();
}
#endif

}  // namespace perf
}  // namespace restart_rollback
}  // namespace register_sgx
