#include "perf.h"

extern ssize_t my_idx;

namespace paxos_sgx {
namespace restart_rollback {
namespace perf {

#ifndef INTRUSIVE_PERF
void* perf_stats::data() { return nullptr; }
size_t perf_stats::size() { return 0; }
void perf_stats::print(FILE* stream) {}

void perf_recorder::add(std::string const& name, double measurement) {}
void perf_recorder::add(std::string const& name) {}
#else

void* perf_stats::data() {
    return reinterpret_cast<void*>(this->measures.data());
}
size_t perf_stats::size() { return this->measures.size(); }

void perf_stats::print(FILE* stream) {
    fprintf(stream, " == [STATS %zd] \n", my_idx);
    for (auto const& m : this->measures) {
        switch (m.type()) {
            case measure_type::FULL:
                if (m.count() > 0) {
                    fprintf(stream,
                            "%s\t| count = %lu | avg = %.3lf | range = %.3lf - "
                            "%.3lf | sum = %.3lf\n",
                            find_name(m.id()).c_str(), m.count(), m.avg(),
                            m.min(), m.max(), m.sum());
                } else {
                    fprintf(stream, "%s\t| count = %lu\n",
                            find_name(m.id()).c_str(), m.count());
                }
                break;

            case measure_type::COUNT:
                fprintf(stream, "%s\t| count = %lu\n",
                        find_name(m.id()).c_str(), m.count());
        }
    }
    fprintf(stream, " == [STATS %zd] \n", my_idx);
    fflush(stream);
}

void perf_recorder::add(std::string const& name, double measurement) {
    int const id = name_map.at(name);
    auto res = std::find_if(this->measure_ptr, this->measure_ptr + this->size,
                            [id](auto const& m) { return m.id() == id; });

    if (res == this->measure_ptr + this->size) {
        ERROR("Name %s not found in perf measures", name.c_str());
    } else {
        res->add(measurement);
    }
}

void perf_recorder::add(std::string const& name) {
    int const id = name_map.at(name);
    auto res = std::find_if(this->measure_ptr, this->measure_ptr + this->size,
                            [id](auto const& m) { return m.id() == id; });

    if (res == this->measure_ptr + this->size) {
        ERROR("Name %s not found in perf measures", name.c_str());
    } else {
        res->add();
    }
}
#endif

}  // namespace perf
}  // namespace restart_rollback
}  // namespace paxos_sgx
