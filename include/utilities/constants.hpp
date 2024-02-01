#ifndef INCLUDE_UTILITIES_CONSTANTS_HPP_
#define INCLUDE_UTILITIES_CONSTANTS_HPP_

#include <matplot/matplot.h>

#include <string>
#include <chrono>
#include <unordered_map>

const int NUM_AIRDROP_BOTTLES = 5;

const int DEFAULT_GCS_PORT = 5010;

const char MISSION_CONFIG_PATH[] = "./mission-config.json";

const matplot::color FLIGHT_BOUND_COLOR = matplot::color::red;
const matplot::color AIRDROP_BOUND_COLOR = matplot::color::blue;
const matplot::color WAYPOINTS_COLOR = matplot::color::yellow;

const std::chrono::milliseconds MISSION_PREP_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds PATH_GEN_TICK_WAIT = std::chrono::milliseconds(1000);
const std::chrono::milliseconds TAKEOFF_PREP_TICK_WAIT = std::chrono::milliseconds(1000);

const double EARTH_RADIUS = 6378137.0;

enum HTTPStatus {
    OK = 200,

    BAD_REQUEST = 400,
    NOT_FOUND = 404,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
};

#define _SET_HTTP_MAPPING(msg) {HTTPStatus::msg, #msg}
const std::unordered_map<HTTPStatus, const char*> HTTP_STATUS_TO_STRING = {
    _SET_HTTP_MAPPING(OK),
    _SET_HTTP_MAPPING(BAD_REQUEST), _SET_HTTP_MAPPING(NOT_FOUND),
    _SET_HTTP_MAPPING(INTERNAL_SERVER_ERROR), _SET_HTTP_MAPPING(NOT_IMPLEMENTED)
};

namespace mime {
    const char json[] = "application/json";
    const char plaintext[] = "text/plain";
}

#endif  // INCLUDE_UTILITIES_CONSTANTS_HPP_
