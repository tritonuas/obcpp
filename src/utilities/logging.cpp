#include "utilities/logging.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <filesystem>

time_t getUnixTime() {
    using namespace std::chrono;

    const auto now = system_clock::now();
    return duration_cast<seconds>(now.time_since_epoch()).count();
}

/*
 * Gets the correct relative filepath to save the log
 * in a logs/ directory at the root level of the repository.
 * 
 * Dependent on the fact that the executable is at obcpp/build/bin/obcpp
 */
std::string getLoggingFilename(int argc, char* argv[]) {
    auto executable_path = std::filesystem::absolute(argv[0]).lexically_normal();
    auto repo_root_path = executable_path.parent_path().parent_path().parent_path();

    // TODO: update the container to have G++ 13, which
    // enables support for std::format so we don't have
    // to do stream bullshit just to format a string
    // Also can just convert getUnixTime() into getTimeString()
    std::ostringstream sstream;
    sstream <<  repo_root_path.string() << "/logs/" << getUnixTime() << ".log";
    return sstream.str();
}