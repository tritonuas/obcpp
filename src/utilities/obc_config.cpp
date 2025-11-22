#include "utilities/obc_config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config_macros.hpp"

OBCConfig::OBCConfig(int argc, char* argv[]) {
    // If config-json name is passed in
    if (argc != 5) {
        LOG_F(ERROR, "INVALID COMMAND LINE ARGUMENTS");
        LOG_F(
            ERROR,
            "Expected use: ./obcpp [config_dir] [rel_config_path] [plane_name] [flight_type]");  // NOLINT
        LOG_F(ERROR,
              "Example 1 (Test Flight with Jetson): bin/obcpp ../configs jetson stickbug "
              "test-flight");  // NOLINT
        LOG_F(
            ERROR,
            "Example 2 (Local Testing with SITL): bin/obcpp ../configs dev stickbug sitl");  // NOLINT
        LOG_F(ERROR, "For more help, check the README");
        LOG_F(FATAL, "ABORTING...");
    }

    std::string configs_dir = argv[1];
    std::string config_file = std::string(argv[2]) + ".json";
    std::string plane_name = argv[3];
    std::string flight_type_file = std::string(argv[4]) + ".json";

    auto config_file_path = std::filesystem::path(configs_dir) / config_file;
    auto params_dir = std::filesystem::path(configs_dir) / "params" / plane_name;

    // Load in json file
    std::ifstream configStream(config_file_path);
    if (!configStream.is_open()) {
        throw std::invalid_argument(std::string("Invalid path to config file: ") +
                                    std::string(argv[1]));
    }
    // the macros expect this to be called "configs" so don't change it without also
    // changing the macros
    nlohmann::json configs = nlohmann::json::parse(configStream, nullptr, true, true);

    // Read this in first before anything else so that all of the read in values get logged
    // to the config file. Otherwise they will be output to the terminal but not saved to
    // the file.
    SET_CONFIG_OPT(logging, dir);
    initLogging(this->logging.dir, true, argc, argv);

    SET_CONFIG_OPT(network, mavlink, connect);
    SET_CONFIG_OPT(network, mavlink, log_params);
    SET_CONFIG_OPT(network, mavlink, telem_poll_rate);
    SET_CONFIG_OPT(network, gcs, port);

    SET_CONFIG_OPT(pathing, laps);
    SET_CONFIG_OPT(pathing, rrt, iterations_per_waypoint);
    SET_CONFIG_OPT(pathing, rrt, rewire_radius);
    SET_CONFIG_OPT(pathing, rrt, optimize);
    SET_CONFIG_OPT(pathing, rrt, generate_deviations);

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

    SET_CONFIG_OPT(cv, yolo_model_dir);
    SET_CONFIG_OPT(cv, detection_threshold);
    SET_CONFIG_OPT_VARIANT(AirdropDropMethod, pathing, approach, drop_method);
    SET_CONFIG_OPT(pathing, approach, drop_angle_rad);
    SET_CONFIG_OPT(pathing, approach, drop_altitude_m);
    SET_CONFIG_OPT(pathing, approach, guided_drop_distance_m);
    SET_CONFIG_OPT(pathing, approach, unguided_drop_distance_m);

    SET_CONFIG_OPT(pathing, dubins, turning_radius);
    SET_CONFIG_OPT(pathing, dubins, point_separation);

    SET_CONFIG_OPT(camera, type);
    SET_CONFIG_OPT(camera, save_dir);
    SET_CONFIG_OPT(camera, save_images_to_file);
    SET_CONFIG_OPT(camera, mock, not_stolen_port);
    SET_CONFIG_OPT(camera, mock, runway);
    SET_CONFIG_OPT(camera, mock, num_targets);
    SET_CONFIG_OPT(camera, mock, connection_timeout);

    SET_CONFIG_OPT(takeoff, altitude_m);
    SET_CONFIG_OPT(takeoff, payload_size);

    std::string common_params_path = params_dir / "common.json";
    std::ifstream common_params_stream(common_params_path);
    if (!common_params_stream.is_open()) {
        LOG_F(FATAL, "Invalid path to common params file: %s (Does this file exist?)",
              common_params_path.c_str());
    }
    std::string specific_params_path = params_dir / flight_type_file;
    std::ifstream specific_params_stream(specific_params_path);
    if (!specific_params_stream.is_open()) {
        LOG_F(FATAL, "Invalid path to specific params file: %s (Does this file exist?)",
              specific_params_path.c_str());
    }

    auto common_params = nlohmann::json::parse(common_params_stream, nullptr, true, true);
    auto specific_params = nlohmann::json::parse(specific_params_stream, nullptr, true, true);

    for (const auto& [param, val] : common_params.items()) {
        this->mavlink_parameters.param_map.insert({param, val});
    }

    for (const auto& [param, val] : specific_params.items()) {
        // specifically using [] syntax here so that it overwrites any params that
        // were previously set in the common file
        this->mavlink_parameters.param_map[param] = val;
    }
}
