#ifndef INCLUDE_UTILITIES_LOGGING_HPP_
#define INCLUDE_UTILITIES_LOGGING_HPP_

#include <string>
#include <utility>

// Note: for any of these custom logging levels you have to use VLOG_F
// instead of LOG_F, so the integer value will be able to be passed
// into the macro correctly

// Logging levels for mav sdk mesages
#define MAV_INFO 1
#define MAV_WARN 2
#define MAV_ERR  3

// For any finer grained logging messages we might need to add
#define TRACE 4

time_t getUnixTime();

std::pair<std::string,std::string> getLoggingFilenames(int argc, char* argv[]);

void initLogging(int argc, char* argv[]);

#endif  // INCLUDE_UTILITIES_LOGGING_HPP_
