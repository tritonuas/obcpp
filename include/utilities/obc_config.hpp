#ifndef INCLUDE_UTILITIES_OBC_CONFIG_HPP_
#define INCLUDE_UTILITIES_OBC_CONFIG_HPP_

#include <string>
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

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

    struct {
        float altitude_m;
    } takeoff;

    RRTConfig rrt_config;
    AirdropSearchConfig coverage_pathing_config;
    AirdropApproachConfig airdrop_pathing_config;
    CameraConfig camera_config;

    // Load user specified config json, or make a new one
    OBCConfig(int argc, char* argv[]);

 private:
    const std::string configsPath = "/workspaces/obcpp/configs/";

    void makeDefault();
};

#endif  // INCLUDE_UTILITIES_OBC_CONFIG_HPP_
