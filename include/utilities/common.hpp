#ifndef INCLUDE_UTILITIES_COMMON_HPP_
#define INCLUDE_UTILITIES_COMMON_HPP_

#include <chrono>
#include <utility>
#include <vector>

// #include <gen_protos/protos/obc.pb.h>
#include "protos/obc.pb.h"


std::chrono::seconds getUnixTime_s();
std::chrono::milliseconds getUnixTime_ms();

int checkCounterClockwise(std::vector<GPSCoord> coords); 

#endif  // INCLUDE_UTILITIES_COMMON_HPP_
