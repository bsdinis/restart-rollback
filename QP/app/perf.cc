#include "perf.h"

extern ssize_t my_idx;

namespace epidemics {
namespace basicQP {
namespace perf {

// implemented inside SGX only
void perf_recorder::add(std::string const& name, double measurement) {}
void perf_recorder::add(std::string const& name) {}

#ifndef INTRUSIVE_PERF
void* perf_stats::data() { return nullptr; }
size_t perf_stats::size() { return 0; }
void perf_stats::print(FILE* stream) {}
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
#endif

}  // namespace perf
}  // namespace basicQP
}  // namespace epidemics
