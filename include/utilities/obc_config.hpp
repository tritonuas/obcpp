#ifndef INCLUDE_UTILITIES_OBC_CONFIG_HPP_
#define INCLUDE_UTILITIES_OBC_CONFIG_HPP_

#include <string>

struct OBCConfig {
    struct {
        std::string dir;
    } logging;

    struct {
        struct {
            int port;
        } gcs;
        struct {
            std::string connect;
        } mavlink;
    } network;

    // Load user specified config json, or make a new one
    OBCConfig(int argc, char* argv[]);

 private:
    const std::string configsPath = "/workspaces/obcpp/configs/";

    void makeDefault();
};

#endif  // INCLUDE_UTILITIES_OBC_CONFIG_HPP_
