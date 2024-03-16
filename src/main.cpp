#include <chrono>
#include <string>
#include <fstream>

#include "nlohmann/json.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    // TODO: pull logging folder from config
    initLogging("/workspaces/obcpp/logs", true, argc, argv);

    // START: My Code 
    LOG_F(INFO, "HEEEELLLLLOOOOOOOOO");

    // Load user specified config json
    std::string fileName = argv[1];
    std::ifstream configStream("/workspaces/obcpp/configs/" + fileName);
    json config = json::parse(configStream);

    // Get values
    std::string mavlinkIP = config["network"]["mavlink"]["connect"];
    int gcsPORTInt = config["network"]["gcs"]["port"];
    LOG_F(INFO, mavlinkIP.c_str());
    LOG_F(INFO, std::to_string(gcsPORTInt).c_str());

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
