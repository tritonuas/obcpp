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

    // Load user specified config json, or make a new one
    json config;
    try{
        std::string fileName = argv[1];
        std::ifstream configStream("/workspaces/obcpp/configs/" + fileName);
        config = json::parse(configStream);
    } catch (...){
        config["network"]["mavlink"]["connect"] = "tcp://172.17.0.1:5760";
        config["network"]["gcs"]["port"] = 5010;
        std::ofstream configFile("/workspaces/obcpp/configs/default-config.json");
        configFile << config.dump(4);
    }

    // Get values
    std::string mavlinkURL = config["network"]["mavlink"]["connect"];
    int gcsPORTInt = config["network"]["gcs"]["port"];
    LOG_F(INFO, mavlinkURL.c_str());
    LOG_F(INFO, std::to_string(gcsPORTInt).c_str());
    // END: My Code

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(mavlinkURL.c_str(), gcsPORTInt);
    obc.run();
}
