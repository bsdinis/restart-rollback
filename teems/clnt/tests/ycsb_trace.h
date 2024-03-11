#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <sstream>

#include "teems.h"
#include "untrusted.h"
#include "log.h"

namespace teems{
namespace ycsb{

enum class OpType {
    Get,
    Put,
    Del,
    Think
};

class Operation {
    public:
    static Operation CreateGet(int64_t key) {
        return Operation(OpType::Get, 0.0, key, 0);
    }
    static Operation CreatePut(int64_t key, size_t size) {
        return Operation(OpType::Put, 0.0, key, size);
    }
    static Operation CreateDel(int64_t key) {
        return Operation(OpType::Del, 0.0, key, 0);
    }
    static Operation CreateThink(double thinktime) {
        return Operation(OpType::Think, thinktime, 0, 0);
    }

    void do_teems_operation() {
        do_operation(false);
    }
    void do_untrusted_operation() {
        do_operation(true);
    }

    private:
    void do_operation(bool untrusted_store_only) {
        auto const start = std::chrono::system_clock::now();
        int64_t policy_version;
        int64_t timestamp;
        std::vector<uint8_t> value;
        timespec req;
        timespec rem;
        switch (m_type) {
            case OpType::Get:
                if (untrusted_store_only) {
                    teems::untrusted_get(teems::gen_teems_ticket(call_type::Sync), 0, true, std::to_string(m_key), value);
                } else {
                    teems::get(m_key, value, policy_version, timestamp);
                }
                break;
            case OpType::Put:
                value = std::vector<uint8_t>(m_value_size);
                if (untrusted_store_only) {
                    teems::untrusted_put(teems::gen_teems_ticket(call_type::Sync), 0, true, std::to_string(m_key), value);
                } else {
                    teems::put(m_key, value, policy_version, timestamp);
                }
                break;
            case OpType::Del:
                if (untrusted_store_only) {
                    teems::untrusted_put(teems::gen_teems_ticket(call_type::Sync), 0, true, std::to_string(m_key), value);
                } else {
                    teems::put(m_key, value, policy_version, timestamp);
                }
                break;
            case OpType::Think:
                time_t usec = (time_t) m_thinktime;
                req.tv_sec = usec / 1000000;
                req.tv_nsec = (time_t) ((m_thinktime - usec) * 100);

                while (nanosleep(&req, &rem) != 0) {
                    perror("nanosleep");
                    req.tv_sec = rem.tv_sec;
                    req.tv_nsec = rem.tv_nsec;
                }
                break;
        }

        auto const end = std::chrono::system_clock::now();
        auto const elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        switch (m_type) {
            case OpType::Get:
                fprintf(stdout, "get, %ld\n", elapsed.count());
                break;
            case OpType::Put:
                fprintf(stdout, "put, %ld\n", elapsed.count());
                break;
            case OpType::Del:
                fprintf(stdout, "del, %ld\n", elapsed.count());
                break;
            case OpType::Think:
                break;
        }
    }

    Operation(OpType type, double thinktime, int64_t key, size_t size) :
     m_type(type), m_thinktime(thinktime), m_key(key), m_value_size(size) {}

    OpType m_type = OpType::Think;
    double m_thinktime = 0.0;
    int64_t m_key = 0;
    size_t m_value_size = 0;
};

namespace {

Operation read_op(std::string line) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    while ((pos = line.find(",")) != std::string::npos) {
        tokens.emplace_back(line.substr(0, pos));
        line.erase(0, pos + 1); // len(",") == 1
    }

    if (tokens[0] == "w") {
        int64_t key = 0;
        size_t size = 0;

        std::stringstream key_stream(tokens[1]);
        key_stream >> key;

        std::stringstream size_stream(tokens[2]);
        size_stream >> size;
        return Operation::CreatePut(key, size);
    } else if (tokens[0] == "r") {
        int64_t key = 0;
        std::stringstream key_stream(tokens[1]);
        key_stream >> key;
        return Operation::CreateGet(key);
    } else if (tokens[0] == "d") {
        int64_t key = 0;
        std::stringstream key_stream(tokens[1]);
        key_stream >> key;
        return Operation::CreateDel(key);
    } else if (tokens[0] == "t") {
        double thinktime = 0.0;
        std::stringstream thinktime_stream(tokens[1]);
        thinktime_stream >> thinktime;
        return Operation::CreateThink(thinktime);
    }

    KILL("error: %s", tokens[0].c_str());
}

std::vector<Operation> import_trace(std::string filename) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit);
    file.open(filename);
    file.exceptions(std::ifstream::goodbit);

    std::vector<Operation> trace;
    std::string line;
    while (std::getline(file, line)) {
        trace.emplace_back(read_op(line));
    }

    return trace;
}
} // anonymous namespace

} //ycsb
} //teems
