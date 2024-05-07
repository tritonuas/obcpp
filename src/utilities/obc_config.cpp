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

        this->rrt_config.iterations_per_waypoint =
            configs["pathing"]["rrt"]["iterations_per_waypoint"];
        this->rrt_config.rewire_radius = configs["pathing"]["rrt"]["rewire_radius"];
        this->rrt_config.optimize = configs["pathing"]["rrt"]["optimize"];
        this->rrt_config.point_fetch_method = configs["pathing"]["rrt"]["point_fetch_methods"];
        this->rrt_config.allowed_to_skip_waypoints =
            configs["pathing"]["rrt"]["allowed_to_skip_waypoints"];

        this->coverage_pathing_config.optimize = configs["pathing"]["coverage"]["optimize"];
        this->coverage_pathing_config.vertical = configs["pathing"]["coverage"]["vertical"];
        std::cout << configs["pathing"]["coverage"]["one_way"] << std::endl;
        this->coverage_pathing_config.one_way = configs["pathing"]["coverage"]["one_way"];

        this->camera_config.type = configs["camera"]["type"];

        this->camera_config.mock.images_dir = configs["camera"]["mock"]["images_dir"];

        this->camera_config.lucid.sensor_shutter_mode =
            configs["camera"]["lucid"]["sensor_shuttle_mode"];

        this->camera_config.lucid.acquisition_frame_rate_enable =
            configs["camera"]["lucid"]["acquisition_frame_rate_enable"];
        this->camera_config.lucid.target_brightness =
            configs["camera"]["lucid"]["target_brightness"];
        this->camera_config.lucid.exposure_auto =
            configs["camera"]["lucid"]["exposure_auto"];
        this->camera_config.lucid.exposure_time =
            configs["camera"]["lucid"]["exposure_time"];
        this->camera_config.lucid.exposure_auto_damping =
            configs["camera"]["lucid"]["exposure_auto_damping"];
        this->camera_config.lucid.exposure_auto_algorithm =
            configs["camera"]["lucid"]["exposure_auto_algorithm"];
        this->camera_config.lucid.exposure_auto_upper_limit =
            configs["camera"]["lucid"]["exposure_auto_upper_limit"];
        this->camera_config.lucid.exposure_auto_lower_limit =
            configs["camera"]["lucid"]["exposure_auto_lower_limit"];

        this->camera_config.lucid.stream_auto_negotiate_packet_size =
            configs["camera"]["lucid"]["stream_auto_negotiate_packet_size"];
        this->camera_config.lucid.stream_packet_resend_enable =
            configs["camera"]["lucid"]["stream_packet_resend_enable"];

        this->camera_config.lucid.device_link_throughput_limit_mode =
            configs["camera"]["lucid"]["device_link_throughput_limit_mode"];
        this->camera_config.lucid.device_link_throughput_limit =
            configs["camera"]["lucid"]["device_link_throughput_limit"];

        this->camera_config.lucid.gamma_enable = configs["camera"]["lucid"]["gamma_enable"];
        this->camera_config.lucid.gamma = configs["camera"]["lucid"]["gamma"];
        this->camera_config.lucid.gain_auto = configs["camera"]["lucid"]["gain_auto"];
        this->camera_config.lucid.gain_auto_upper_limit =
            configs["camera"]["lucid"]["gain_auto_upper_limit"];
        this->camera_config.lucid.gain_auto_lower_limit =
            configs["camera"]["lucid"]["gain_auto_lower_limit"];

        this->takeoff.m_altitude = configs["takeoff"]["m_altitude"];
    } else {
        makeDefault();
    }
}

void OBCConfig::makeDefault() {
    // Set configs
    this->logging.dir = "/workspaces/obcpp/logs";
    this->network.mavlink.connect = "tcp://172.17.0.1:5760";
    this->network.gcs.port = 5010;

    this->rrt_config.iterations_per_waypoint = ITERATIONS_PER_WAYPOINT;
    this->rrt_config.rewire_radius = REWIRE_RADIUS;
    this->rrt_config.optimize = true;
    this->rrt_config.point_fetch_method = NEAREST;
    this->rrt_config.allowed_to_skip_waypoints = false;

    this->coverage_pathing_config.optimize = true;
    this->coverage_pathing_config.vertical = false;
    this->coverage_pathing_config.one_way = false;

    this->camera_config.type = "mock";

    this->camera_config.mock.images_dir = "/workspaces/obcpp/tests/integration/images/saliency/";

    this->camera_config.lucid.sensor_shutter_mode = "Rolling";

    this->camera_config.lucid.acquisition_frame_rate_enable = true;
    this->camera_config.lucid.target_brightness = 70;
    this->camera_config.lucid.exposure_auto = "Continuous";
    this->camera_config.lucid.exposure_time = 3000;
    this->camera_config.lucid.exposure_auto_damping = 1;
    this->camera_config.lucid.exposure_auto_algorithm = "Median";
    this->camera_config.lucid.exposure_auto_upper_limit = 500;
    this->camera_config.lucid.exposure_auto_lower_limit = 360;

    this->camera_config.lucid.stream_auto_negotiate_packet_size = true;
    this->camera_config.lucid.stream_packet_resend_enable = true;

    this->camera_config.lucid.device_link_throughput_limit_mode = "On";
    this->camera_config.lucid.device_link_throughput_limit = 125000000;

    this->camera_config.lucid.gamma_enable = true;
    this->camera_config.lucid.gamma = 0.5;
    this->camera_config.lucid.gain_auto = "Continuous";
    this->camera_config.lucid.gain_auto_upper_limit = 10;
    this->camera_config.lucid.gain_auto_lower_limit = 1;

    this->takeoff.m_altitude = 30.0;
}
