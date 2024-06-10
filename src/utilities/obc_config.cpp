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

using json = nlohmann::json;

OBCConfig::OBCConfig(int argc, char* argv[]) {
    // If config-json name is passed in
    if (argc > 1) {
        // Load in json file
        std::ifstream configStream(argv[1]);
        if (!configStream.is_open()) {
            throw std::invalid_argument(std::string("Invalid path to config file: ") +
                                        std::string(argv[1]));
        }
        json configs = json::parse(configStream);

        // Set configs
        this->logging.dir = configs["logging"]["dir"];
        this->network.mavlink.connect = configs["network"]["mavlink"]["connect"];
        this->network.gcs.port = configs["network"]["gcs"]["port"];

        this->pathing.rrt.iterations_per_waypoint =
            configs["pathing"]["rrt"]["iterations_per_waypoint"];
        this->pathing.rrt.rewire_radius = configs["pathing"]["rrt"]["rewire_radius"];
        this->pathing.rrt.optimize = configs["pathing"]["rrt"]["optimize"];
        this->pathing.rrt.point_fetch_method = configs["pathing"]["rrt"]["point_fetch_methods"];
        this->pathing.rrt.allowed_to_skip_waypoints =
            configs["pathing"]["rrt"]["allowed_to_skip_waypoints"];

        this->pathing.coverage.coverage_altitude_m =
            configs["pathing"]["coverage"]["coverage_altitude_m"];
        this->pathing.coverage.optimize = configs["pathing"]["coverage"]["optimize"];
        this->pathing.coverage.vertical = configs["pathing"]["coverage"]["vertical"];
        this->pathing.coverage.one_way = configs["pathing"]["coverage"]["one_way"];

        this->cv.matching_model_dir = configs["cv"]["matching_model_dir"];
        this->cv.segmentation_model_dir = configs["cv"]["segmentation_model_dir"];
        this->cv.saliency_model_dir = configs["cv"]["saliency_model_dir"];

        this->pathing.approach.drop_method = configs["pathing"]["approach"]["drop_method"];
        this->pathing.approach.drop_angle_rad =
            configs["pathing"]["approach"]["drop_angle_rad"];
        this->pathing.approach.drop_altitude_m =
            configs["pathing"]["approach"]["drop_altitude_m"];
        this->pathing.approach.guided_drop_distance_m =
            configs["pathing"]["approach"]["guided_drop_distance_m"];
        this->pathing.approach.unguided_drop_distance_m =
            configs["pathing"]["approach"]["unguided_drop_distance_m"];

        this->camera.type = configs["camera"]["type"];
        this->camera.save_dir = configs["camera"]["save_dir"];
        this->camera.mock.images_dir = configs["camera"]["mock"]["images_dir"];

        this->camera.lucid.sensor_shutter_mode =
            configs["camera"]["lucid"]["sensor_shuttle_mode"];
        this->camera.lucid.acquisition_frame_rate_enable =
            configs["camera"]["lucid"]["acquisition_frame_rate_enable"];
        this->camera.lucid.target_brightness =
            configs["camera"]["lucid"]["target_brightness"];
        this->camera.lucid.exposure_auto = configs["camera"]["lucid"]["exposure_auto"];
        this->camera.lucid.exposure_time = configs["camera"]["lucid"]["exposure_time"];
        this->camera.lucid.exposure_auto_damping =
            configs["camera"]["lucid"]["exposure_auto_damping"];
        this->camera.lucid.exposure_auto_algorithm =
            configs["camera"]["lucid"]["exposure_auto_algorithm"];
        this->camera.lucid.exposure_auto_upper_limit =
            configs["camera"]["lucid"]["exposure_auto_upper_limit"];
        this->camera.lucid.exposure_auto_lower_limit =
            configs["camera"]["lucid"]["exposure_auto_lower_limit"];

        this->camera.lucid.stream_auto_negotiate_packet_size =
            configs["camera"]["lucid"]["stream_auto_negotiate_packet_size"];
        this->camera.lucid.stream_packet_resend_enable =
            configs["camera"]["lucid"]["stream_packet_resend_enable"];

        this->camera.lucid.device_link_throughput_limit_mode =
            configs["camera"]["lucid"]["device_link_throughput_limit_mode"];
        this->camera.lucid.device_link_throughput_limit =
            configs["camera"]["lucid"]["device_link_throughput_limit"];

        this->camera.lucid.gamma_enable = configs["camera"]["lucid"]["gamma_enable"];
        this->camera.lucid.gamma = configs["camera"]["lucid"]["gamma"];
        this->camera.lucid.gain_auto = configs["camera"]["lucid"]["gain_auto"];
        this->camera.lucid.gain_auto_upper_limit =
            configs["camera"]["lucid"]["gain_auto_upper_limit"];
        this->camera.lucid.gain_auto_lower_limit =
            configs["camera"]["lucid"]["gain_auto_lower_limit"];

        this->takeoff.altitude_m = configs["takeoff"]["altitude_m"];
    } else {
        LOG_F(FATAL, "You must specify a config file. e.g. bin/obcpp ../configs/dev-config.json");
    }
}
