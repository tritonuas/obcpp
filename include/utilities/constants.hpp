#ifndef INCLUDE_UTILITIES_CONSTANTS_HPP_
#define INCLUDE_UTILITIES_CONSTANTS_HPP_

#include <string>
#include <chrono>

#include <matplot/matplot.h>

// common ratios of pi
const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2;

// FOR TEST, VALUES DONT MAKE SENSE
const double TURNING_RADIUS = 5;
const double POINT_SEPARATION = 1;

const int DEFAULT_GCS_PORT = 5010;

const int NUM_AIRDROP_BOTTLES = 5;

const int DEFAULT_GCS_PORT = 5010;

const char MISSION_CONFIG_PATH[] = "./mission-config.json";

const matplot::color FLIGHT_BOUND_COLOR = matplot::color::red;
const matplot::color AIRDROP_BOUND_COLOR = matplot::color::blue;
const matplot::color WAYPOINTS_COLOR = matplot::color::yellow;

const std::chrono::milliseconds MISSION_PREP_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds PATH_GEN_TICK_WAIT = std::chrono::milliseconds(1000);

#endif  // INCLUDE_UTILITIES_CONSTANTS_HPP_
