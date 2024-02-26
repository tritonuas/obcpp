#ifndef INCLUDE_UTILITIES_CONSTANTS_HPP_
#define INCLUDE_UTILITIES_CONSTANTS_HPP_

#include <matplot/matplot.h>

#include <chrono>
#include <string>
#include <unordered_map>

// common ratios of pi
const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2;

// FOR TEST, VALUES DONT MAKE SENSE
const double TURNING_RADIUS = 30;
const double POINT_SEPARATION = 20;

const int TRIES_FOR_RANDOM_POINT = 64;  // for generating random points
const int MAX_DUBINS_OPTIONS_TO_PARSE = 256;

const double LARGE_COST = 999999999999.0;

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
const std::chrono::milliseconds MISSION_UPLOAD_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds MISSION_START_TICK_WAIT = std::chrono::milliseconds(100);

const double EARTH_RADIUS_METERS = 6378137.0;

#endif  // INCLUDE_UTILITIES_CONSTANTS_HPP_
