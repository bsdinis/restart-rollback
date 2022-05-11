#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include "log.h"
#include "teems.h"

using namespace teems;  // namespace sanity

namespace {
std::string global_config_path = "../server/default.conf";
ssize_t global_index = 0;
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    if (init(global_config_path.c_str(), global_index) == -1) {
        INFO("could not connect [config = %s, idx = %zd]",
             global_config_path.c_str(), global_index);
        return -1;
    }
    close(true);

    return 0;
}

namespace {
void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n",
            global_config_path.c_str());
    fprintf(stderr, "\t-i [index = %zd]\n", global_index);
    fprintf(stderr, "\t-h : print help message\n");
}

// cli
void parse_cli_args(int argc, char** argv) {
    opterr = 0;  // ignore default error
    int opt;

    while ((opt = getopt(argc, argv, "c:i:h")) != -1) {
        switch (opt) {
            case 'c':
                global_config_path = std::string(optarg, strlen(optarg));
                break;

            case 'i':
                errno = 0;
                global_index = std::stoll(optarg);
                if (errno != 0) {
                    fprintf(stderr, "Failed to parse %s as integer: %s\n",
                            optarg, strerror(errno));
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;

            default: /* '?' */
                ERROR("Unknown flag -%c", optopt);
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

}  // anonymous namespace
