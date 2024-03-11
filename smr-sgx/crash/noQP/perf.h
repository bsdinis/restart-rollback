#ifndef _PERF_APP_STATS_
#define _PERF_APP_STATS_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#include <map>
#include <algorithm>
#include <limits>
#include "log.h"

namespace paxos_sgx {
namespace crash {
namespace perf {

namespace {

// XXX: CHANGE ME
// add new operations (ints must be unique!)
std::map<std::string, int> name_map {
    {"sum", 1},
    {"ping", 2},
};

std::string find_name(int id) {
    auto result = std::find_if(
              std::begin(name_map),
              std::end(name_map),
              [id](const auto& p) {return p.second == id; });

    return result != name_map.end() ? result->first : "id not found!";
}

} // anonymous namespace

enum class measure_type {
    FULL,   // min, max, sum, average
    COUNT,  // count
};

class measure {
   public:
    measure(std::string const& name, measure_type t = measure_type::FULL) : type_(t), id_(name_map.at(name)) {
    }

    inline measure_type type() const { return this->type_; }
    inline int64_t count() const { return this->count_; }
    inline double avg() const {
        return this->count_ > 0 ? this->sum_ / static_cast<double>(this->count_) : 0;
    }
    inline double sum() const { return this->sum_; }
    inline double min() const { return this->min_; }
    inline double max() const { return this->max_; }
    inline int    id()  const { return this->id_; }
    inline std::string name() const { return find_name(this->id_); }

    inline void   add(double const m) {
        this->count_++;
        if (this->type_ == measure_type::FULL) {
            this->sum_ += m;
            this->min_ = std::min(this->min_, m);
            this->max_ = std::max(this->max_, m);
        }
    }

    inline void   add() {
        this->count_++;
        if (this->type_ == measure_type::FULL) {
            ERROR("add requires an argument when the measure is FULL");
        }
    }

   private:
    measure_type type_ = measure_type::FULL;
    double sum_ = 0;
    double min_ = std::numeric_limits<double>::max();
    double max_ = std::numeric_limits<double>::min();
    int64_t count_ = 0;
    int id_;
};

class perf_stats {
   public:
#ifdef INTRUSIVE_PERF
    perf_stats(std::initializer_list<measure> l) : measures(l) {}
#else
    perf_stats(std::initializer_list<measure> l) {}
#endif
    void* data();              // retrieves internally used buffer
    size_t size();             // retrieves internally used buffer
#if defined (__FILE_defined)   // add more as needed
    void print(FILE* stream);  // prints to the stream
#endif

   private:
    std::vector<measure> measures;
};

class perf_recorder {
   public:
    perf_recorder() = default;
#ifdef INTRUSIVE_PERF
    perf_recorder(void * buffer, size_t size) :
        measure_ptr(reinterpret_cast<measure *>(buffer)), size(size) {}
#else
    perf_recorder(void * buffer, size_t size) {}
#endif

    void add(std::string const & name, double measurement);
    // only for COUNT
    void add(std::string const & name);
   private:
#ifdef INTRUSIVE_PERF
    measure * measure_ptr = nullptr;
    size_t size = 0;
#endif
};

} // perf
} // basicQP
} // epidemics

#endif  // _PERF_APP_STATS_
