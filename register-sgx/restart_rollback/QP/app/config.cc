/**
 * config.cc
 *
 * implementation
 */

#include "config.h"
#include "log.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

struct config_node {
    std::string addr;
    int port;
    bool fresh;

    explicit config_node(std::string &line) {
        this->addr = line.substr(0, line.find(" "));
        line.erase(0, line.find(" ") + 1);
        this->port = std::stoi(line.substr(0, line.find(" ")));
        line.erase(0, line.find(" ") + 1);
        this->fresh = !line.substr(0, line.find(" ")).empty();
    }
};

struct config {
    std::vector<config_node> nodes;
    size_t r;
    size_t f;

    explicit config(std::string const &filename) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
        file.exceptions(std::ifstream::goodbit);
        std::string line;

        if (!std::getline(file, line)) {
            KILL("failed to parse config");
        }

        this->r = std::stoi(line.substr(0, line.find(" ")));
        line.erase(0, line.find(" ") + 1);
        this->f = std::stoi(line.substr(0, line.find(" ")));

        while (std::getline(file, line)) {
            this->nodes.emplace_back(line);
        }
    }
};

int config_parse(config_t *const conf, char const *filename) {
    try {
        config cc_config(filename);
        conf->size = cc_config.nodes.size();
        conf->r = cc_config.r;
        conf->f = cc_config.f;
        conf->nodes = (config_node_t *)malloc(sizeof(config_node_t) *
                                              cc_config.nodes.size());
        assert(conf->nodes != nullptr);

        size_t idx = 0;
        for (auto const &node : cc_config.nodes) {
            conf->nodes[idx].port = node.port;
            conf->nodes[idx].addr = strdup(node.addr.c_str());
            conf->nodes[idx].fresh = node.fresh;
            idx++;
        }
    } catch (std::ifstream::failure &) {
        ERROR("failed to open configuration file: %s", filename);
        return -1;
    }

    return 0;
}

void config_free(config_t *conf) {
    if (conf == nullptr) return;
    for (size_t idx = 0; idx < conf->size; idx++) {
        free(conf->nodes[idx].addr);
    }

    free(conf->nodes);
}
