// OBCConfig.hpp
#ifndef OBCCONFIG_HPP
#define OBCCONFIG_HPP

#include <string>

struct OBCConfig{

    // Configs
    std::string network_mavlink_connect;
    int network_gcs_port;

    // Load user specified config json, or make a new one
    OBCConfig(int argc, char* argv[]);

    private:
        const std::string configsPath = "/workspaces/obcpp/configs/";

        void makeDefault();
};

#endif  // OBCCONFIG_HPP