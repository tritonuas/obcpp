#include <chrono>

extern "C" {
    #include "network/airdrop_sockets.h"
}

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"


int main(int argc, char* argv[]) {
    OBCConfig config(argc, argv);

    // TODO: pull logging folder from config
    initLogging(config.logging_dir, true, argc, argv);

    OBC obc(config);
    obc.run();
}
