#include "utilities/logging.hpp"

#include <mavsdk/log_callback.h>

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <utility>

#include "utilities/common.hpp"

/*
 * Gets the correct relative filepath to save the log
 * in a logs/ directory at the root level of the repository.
 * 
 * Dependent on the fact that the executable is at obcpp/build/bin/obcpp
 * 
 * returns the filenames for
 * 
 * 1) the readable logging file
 * 2) the everything logging file
 */
std::pair<std::string, std::string> getLoggingFilenames(int argc, char* argv[]) {
    auto executable_path = std::filesystem::absolute(argv[0]).lexically_normal();
    auto repo_root_path = executable_path.parent_path().parent_path().parent_path();

    // TODO: update the container to have G++ 13, which
    // enables support for std::format so we don't have
    // to do stream bullshit just to format a string
    // Also can just convert getUnixTime() into getTimeString()
    auto time = getUnixTime_s().count();
    std::ostringstream sstream_main;
    sstream_main <<  repo_root_path.string() << "/logs/" << time << ".log";
    std::ostringstream sstream_mav;
    sstream_mav <<  repo_root_path.string() << "/logs/" << time << "_all.log";
    return {sstream_main.str(), sstream_mav.str()};
}

void initLogging(int argc, char* argv[]) {
    loguru::init(argc, argv);
    auto [readable_logs, everything_logs] = getLoggingFilenames(argc, argv);
    loguru::add_file(readable_logs.c_str(),
        loguru::Truncate, loguru::Verbosity_INFO);
    loguru::add_file(everything_logs.c_str(),
        loguru::Truncate, loguru::Verbosity_MAX);

    // Overwrite default mavsdk logging to standard out,
    // and redirect to loguru, but use higher level of logging
    // levels so that they only get written to special mavlink
    // log file, not the normal logging file
    mavsdk::log::subscribe(
        [](mavsdk::log::Level level, const std::string& message,
           const std::string& file, int line) {
        std::string parsed_msg = file + ":" + std::to_string(line) + "// " + message;
        switch (level) {
            case mavsdk::log::Level::Debug:
                // Do nothing
                break;
            case mavsdk::log::Level::Info:
                VLOG_F(MAV_INFO, "%s", parsed_msg.c_str());
                break;
            case mavsdk::log::Level::Warn:
                VLOG_F(MAV_WARN, "%s", parsed_msg.c_str());
                break;
            case mavsdk::log::Level::Err:
                VLOG_F(MAV_ERR, "%s", parsed_msg.c_str());
                break;
        }

        return true;  // never log to standard out
    });
}
