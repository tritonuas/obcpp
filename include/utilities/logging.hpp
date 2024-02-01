#ifndef INCLUDE_UTILITIES_LOGGING_HPP_
#define INCLUDE_UTILITIES_LOGGING_HPP_

#include <string>

time_t getUnixTime();

std::string getLoggingFilename(int argc, char* argv[]);

#endif  // INCLUDE_UTILITIES_LOGGING_HPP_
