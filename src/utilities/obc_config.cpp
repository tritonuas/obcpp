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
        this->logging_dir = configs["logging"]["dir"];
        this->network_mavlink_connect = configs["network"]["mavlink"]["connect"];
        this->network_gcs_port = configs["network"]["gcs"]["port"];
    } else {
        makeDefault();  // Detect if there is already a default-config.json?
    }
}

void OBCConfig::makeDefault() {
    // Set configs
    this->logging_dir = "/workspaces/obcpp/logs";
    this->network_mavlink_connect = "tcp://172.17.0.1:5760";
    this->network_gcs_port = 5010;

    // Create default configs
    json configs;
    configs["logging"]["dir"] = this->logging_dir;
    configs["network"]["mavlink"]["connect"] = this->network_mavlink_connect;
    configs["network"]["gcs"]["port"] = this->network_gcs_port;
    std::ofstream configFile(configsPath + "default-config.json");
    configFile << configs.dump(4);  // Dump to file with 4 space indents
}
