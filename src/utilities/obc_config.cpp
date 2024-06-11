#include "utilities/obc_config.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>

#include "nlohmann/json.hpp"
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config_macros.hpp"

OBCConfig::OBCConfig(int argc, char* argv[]) {
    // If config-json name is passed in
    if (argc != 2) {
        LOG_F(FATAL, "You must specify a config file. e.g. bin/obcpp ../configs/dev-config.json");
    }

    // Load in json file
    std::ifstream configStream(argv[1]);
    if (!configStream.is_open()) {
        throw std::invalid_argument(std::string("Invalid path to config file: ") +
                                    std::string(argv[1]));
    }
    // the macros expect this to be called "configs" so don't change it without also
    // changing the macros
    nlohmann::json configs = nlohmann::json::parse(configStream);

    // Set configs
    SET_CONFIG_OPT(logging, dir);
    SET_CONFIG_OPT(network, mavlink, connect);
    SET_CONFIG_OPT(network, gcs, port);

    SET_CONFIG_OPT(pathing, rrt, iterations_per_waypoint);
    SET_CONFIG_OPT(pathing, rrt, rewire_radius);
    SET_CONFIG_OPT(pathing, rrt, optimize);

    SET_CONFIG_OPT_VARIANT(PointFetchMethod, pathing, rrt, point_fetch_method);

    SET_CONFIG_OPT(pathing, rrt, allowed_to_skip_waypoints);

    SET_CONFIG_OPT_VARIANT(AirdropCoverageMethod, pathing, coverage, method);

    SET_CONFIG_OPT(pathing, coverage, altitude_m);
    SET_CONFIG_OPT(pathing, coverage, camera_vision_m);
    SET_CONFIG_OPT(pathing, coverage, hover, hover_time_s);
    SET_CONFIG_OPT(pathing, coverage, hover, pictures_per_stop);
    SET_CONFIG_OPT(pathing, coverage, forward, optimize);
    SET_CONFIG_OPT(pathing, coverage, forward, vertical);
    SET_CONFIG_OPT(pathing, coverage, forward, one_way);

    SET_CONFIG_OPT(cv, matching_model_dir);
    SET_CONFIG_OPT(cv, segmentation_model_dir);
    SET_CONFIG_OPT(cv, saliency_model_dir);

    SET_CONFIG_OPT_VARIANT(AirdropDropMethod, pathing, approach, drop_method);
    SET_CONFIG_OPT(pathing, approach, drop_angle_rad);
    SET_CONFIG_OPT(pathing, approach, drop_altitude_m);
    SET_CONFIG_OPT(pathing, approach, guided_drop_distance_m);
    SET_CONFIG_OPT(pathing, approach, unguided_drop_distance_m);

    SET_CONFIG_OPT(pathing, dubins, turning_radius);
    SET_CONFIG_OPT(pathing, dubins, point_separation);

    SET_CONFIG_OPT(camera, type);
    SET_CONFIG_OPT(camera, save_dir);
    SET_CONFIG_OPT(camera, mock, images_dir);

    SET_CONFIG_OPT(camera, lucid, sensor_shutter_mode);
    SET_CONFIG_OPT(camera, lucid, acquisition_frame_rate_enable);
    SET_CONFIG_OPT(camera, lucid, target_brightness);
    SET_CONFIG_OPT(camera, lucid, exposure_auto);
    SET_CONFIG_OPT(camera, lucid, exposure_time);
    SET_CONFIG_OPT(camera, lucid, exposure_auto_damping);
    SET_CONFIG_OPT(camera, lucid, exposure_auto_algorithm);
    SET_CONFIG_OPT(camera, lucid, exposure_auto_upper_limit);
    SET_CONFIG_OPT(camera, lucid, exposure_auto_lower_limit);

    SET_CONFIG_OPT(camera, lucid, stream_auto_negotiate_packet_size);
    SET_CONFIG_OPT(camera, lucid, stream_packet_resend_enable);

    SET_CONFIG_OPT(camera, lucid, device_link_throughput_limit_mode);
    SET_CONFIG_OPT(camera, lucid, device_link_throughput_limit);

    SET_CONFIG_OPT(camera, lucid, gamma_enable);
    SET_CONFIG_OPT(camera, lucid, gamma);  // bruh
    SET_CONFIG_OPT(camera, lucid, gain_auto);
    SET_CONFIG_OPT(camera, lucid, gain_auto_upper_limit);
    SET_CONFIG_OPT(camera, lucid, gain_auto_lower_limit);

    SET_CONFIG_OPT(takeoff, altitude_m);
}
