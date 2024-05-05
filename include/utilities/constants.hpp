#ifndef INCLUDE_UTILITIES_CONSTANTS_HPP_
#define INCLUDE_UTILITIES_CONSTANTS_HPP_

#include <matplot/matplot.h>

#include <chrono>
#include <string>
#include <unordered_map>

// Max number of CV pipelines that can be running at the same time
const size_t MAX_CV_PIPELINES = 5;

// common ratios of pi
const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2;

// FROM OBC PYTHON
const double TURNING_RADIUS = 30.0;
const double POINT_SEPARATION = 10.0;

// RRT CONSTANTS
const int ITERATIONS_PER_WAYPOINT = 200;  // number of times RRT is ran per waypoint
const double SEARCH_RADIUS =
    1000.0;  // DOES NOTHING, limits how far off the tree the new node can be
const double REWIRE_RADIUS = 200.0;  // ONLY FOR RRT-STAR, max radius from new node to rewire

// RRT HELPER CONSTANTS
const double EPOCH_TEST_MARGIN = 0.97;        // at what margin of improvement does it stop
const int ENV_PATH_VALIDATION_STEP_SIZE = 5;  // how many points to skip when validating path
const int NUM_EPOCHS = 5;                     // number of times to evaulate the path length
const int K_RANDOM_NODES = 100;               // how many nodes to generate for the tree
const int K_CLOESEST_NODES = 50;  // how many nodes to look at when finding the closest node
const int TOTAL_OPTIONS_FOR_GOAL_CONNECTION =
    2048;  // TODO - MUST SCALE WITH ITERATIONS OR ELSE CANT FIND GOAL

const int TRIES_FOR_RANDOM_POINT = 64;       // for generating random points
const int MAX_DUBINS_OPTIONS_TO_PARSE = 16;  // how many routes to check when connecting two nodes

const int DEFAULT_GCS_PORT = 5010;

const int NUM_AIRDROP_BOTTLES = 5;

const char MISSION_CONFIG_PATH[] = "./mission-config.json";

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

const double EARTH_RADIUS_METERS = 6378137.0;

#endif  // INCLUDE_UTILITIES_CONSTANTS_HPP_
