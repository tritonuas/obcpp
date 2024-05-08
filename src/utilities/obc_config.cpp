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

        this->takeoff.altitude_m = configs["takeoff"]["altitude_m"];
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

    this->takeoff.altitude_m = TAKEOFF_ALTITUDE_M;
}
