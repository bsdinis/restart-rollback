#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include "log.h"
#include "metadata.h"

using namespace teems;  // namespace sanity

namespace {
std::string global_config_path = "../server/default.conf";
void parse_cli_args(int argc, char** argv);
}  // anonymous namespace

int main(int argc, char** argv) {
    parse_cli_args(argc, argv);
    setlinebuf(stdout);

    if (metadata_init(global_config_path.c_str()) == -1) {
        INFO("could not connect [config = %s]", global_config_path.c_str());
        return -1;
    }
    metadata_close(true);

    return 0;
}

namespace {
void usage(char* arg0) {
    fprintf(stderr, "usage: %s\n", basename(arg0));
    fprintf(stderr, "\t-c [configuration file = %s]\n",
            global_config_path.c_str());
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
