#include <chrono>
#include <string>

extern "C" {
    #include "network/airdrop_sockets.h"
}

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"


int main(int argc, char* argv[]) {
    // TODO: pull logging folder from config
    initLogging("/workspaces/obcpp/logs", true, argc, argv);

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
