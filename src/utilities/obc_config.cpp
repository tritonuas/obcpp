#include "utilities/obc_config.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;


OBCConfig::OBCConfig(int argc, char* argv[]) {
    // If config-json name is passed in
    if (argc > 1) {
        // Load in json file
        std::ifstream configStream(configsPath + std::string(argv[1]));
        if (!configStream.is_open()) {
            throw std::invalid_argument("Invalid config-json name");
        }
        json configs = json::parse(configStream);

        // Set configs
        this->logging.dir = configs["logging"]["dir"];
        this->network.mavlink.connect = configs["network"]["mavlink"]["connect"];
        this->network.gcs.port = configs["network"]["gcs"]["port"];
    } else {
        makeDefault();  // Detect if there is already a default-config.json?
    }
}

void OBCConfig::makeDefault() {
    // Set configs
    this->logging.dir = "/workspaces/obcpp/logs";
    this->network.mavlink.connect = "tcp://172.17.0.1:5760";
    this->network.gcs.port = 5010;
}
