#include <chrono>

extern "C" {
    #include "network/airdrop_sockets.h"
}

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"
#include "utilities/OBCConfig.hpp"


int main(int argc, char* argv[]) {
    // TODO: pull logging folder from config
    initLogging("/workspaces/obcpp/logs", true, argc, argv);

    // START: My Code 
    LOG_F(INFO, "HEEEELLLLLOOOOOOOOO");
    OBCConfig config(argc, argv);
    // END: My Code

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(config);
    obc.run();
}
