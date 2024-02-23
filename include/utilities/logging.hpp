#ifndef INCLUDE_UTILITIES_LOGGING_HPP_
#define INCLUDE_UTILITIES_LOGGING_HPP_

#include <string>
#include <utility>

// Include this file instead of loguru.hpp directly
// to ensure that this define is always included before
// the include to loguru.
#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

// Note: for any of these custom logging levels you have to use VLOG_F/S
// instead of LOG_F/S, so the integer value will be able to be passed
// into the macro correctly

// Logging levels for mav sdk mesages
#define MAV_INFO 3
#define MAV_WARN 2
#define MAV_ERR  1

// For any finer grained logging messages we might need to add
#define TRACE 4
#define DEBUG 5


std::pair<std::string, std::string> getLoggingFilenames(int argc, char* argv[]);

void initLogging(int argc, char* argv[]);

#endif  // INCLUDE_UTILITIES_LOGGING_HPP_
