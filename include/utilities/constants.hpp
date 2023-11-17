#ifndef UTILITIES_CONSTANT_HPP_
#define UTILITIES_CONSTANT_HPP_

#include <string>

#include <matplot/matplot.h>

const int NUM_AIRDROP_BOTTLES = 5;

const int DEFAULT_GCS_PORT = 5010;

const std::string MISSION_CONFIG_PATH = "./mission-config.json";

const matplot::color FLIGHT_BOUND_COLOR = matplot::color::red;
const matplot::color AIRDROP_BOUND_COLOR = matplot::color::blue;
const matplot::color WAYPOINTS_COLOR = matplot::color::yellow;

#endif // UTILITIES_CONSTANT_HPP_