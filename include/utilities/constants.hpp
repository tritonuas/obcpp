#ifndef INCLUDE_UTILITIES_CONSTANTS_HPP_
#define INCLUDE_UTILITIES_CONSTANTS_HPP_

#include <matplot/matplot.h>

#include <chrono>
#include <string>
#include <unordered_map>

// Max number of CV pipelines that can be running at the same time.
// NOTE: This number has a large impact on how much memory the OBCpp
// program uses. Each time a new pipeline is loaded into memory,
// all the models and other pipeline state must be allocated. This
// can be on the order of hundreds of megabytes. So, be sure to test
// how much is reasonable to avoid running out of memory on the Jetson.
const size_t MAX_CV_PIPELINES = 2;

// common ratios of pi
const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2;

// RRT CONSTANTS
const int TREE_CAPACITY = 512;  // number of nodes a tree between two waypoints is built out of
// the tree holds the waypoint it is rooted at, a node for every sample RRT takes,
// and the waypoint they end up connecting to
const int ITERATIONS_PER_WAYPOINT = TREE_CAPACITY - 2;  // number of times RRT is ran per waypoint
const double REWIRE_RADIUS = 200.0;  // ONLY FOR RRT-STAR, max radius from new node to rewire

// RRT HELPER CONSTANTS
const int ENV_PATH_VALIDATION_STEP_SIZE = 5;  // how many points to skip when validating path
const int TOTAL_OPTIONS_FOR_GOAL_CONNECTION =
    2048;  // TODO - MUST SCALE WITH ITERATIONS OR ELSE CANT FIND GOAL

const int TRIES_FOR_RANDOM_POINT = 64;       // for generating random points
const int MAX_DUBINS_OPTIONS_TO_PARSE = 16;  // how many routes to check when connecting two nodes

// AIRDROP PATHING
const double WIND_CONST_PER_ALTITUDE = 0.3;  // if I recall correctly, this is 75ft drop stationary
                                             // --> 75ft off with a good amount of wind
const double DROP_ANGLE_RAD = 2.52134317;
const double DROP_ALTITUDE_M = 26.0;
const double GUIDED_DROP_DISTANCE_M = 50.0;
const double UNGUIDED_DROP_DISTANCE_M = 50.0;

// mavlink
const uint16_t WIND_COV = 231;

const int DEFAULT_GCS_PORT = 5010;

const int NUM_AIRDROPS = 2;

const char MISSION_CONFIG_PATH[] = "./mission-config.json";
const double TAKEOFF_ALTITUDE_M = 30.0;
const double COVERAGE_ALTITUDE_M = 30.0;

const matplot::color FLIGHT_BOUND_COLOR = matplot::color::red;
const matplot::color AIRDROP_BOUND_COLOR = matplot::color::magenta;
const matplot::color WAYPOINTS_COLOR = matplot::color::yellow;
const matplot::color TENTATIVE_PATH_COLOR = matplot::color::cyan;
const matplot::color PLANNED_PATH_COLOR = matplot::color::green;

const std::chrono::milliseconds MISSION_PREP_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds PATH_GEN_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds PATH_VALIDATE_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds MAV_UPLOAD_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds TAKEOFF_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds FLY_WAYPOINTS_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds FLY_SEARCH_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds CV_LOITER_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds AIRDROP_PREP_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds AIRDROP_APPROACH_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds MANUAL_LANDING_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds AUTO_LANDING_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds MISSION_DONE_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds ACTIVE_TAKEOFF_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds WAIT_FOR_TAKEOFF_TICK_WAIT = std::chrono::milliseconds(100);
const std::chrono::milliseconds REFUELING_TICK_WAIT = std::chrono::milliseconds(100);

const double EARTH_RADIUS_METERS = 6378137.0;

#endif  // INCLUDE_UTILITIES_CONSTANTS_HPP_
