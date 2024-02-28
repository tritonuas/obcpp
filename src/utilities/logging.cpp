#include "utilities/logging.hpp"

#include <mavsdk/log_callback.h>

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <utility>

#include "utilities/common.hpp"

/*
 * returns the filenames for
 * 
 * 1) the readable logging file
 * 2) the everything logging file
 * 
 * based on the absolute path of the directory in which to save the logs
 * e.g.
 *      "workspaces/obcpp/logs" -> "{workspaces/obcpp/logs/{timestamp}.log,workspaces/obcpp/logs/{timestamp}.log }"
 */
std::pair<std::string, std::string> getLoggingFilenames(std::string directory) {
    auto time = getUnixTime_s().count();
    std::ostringstream sstream_main;
    sstream_main << directory <<"/" << time << ".log";
    std::ostringstream sstream_mav;
    sstream_mav <<  directory << "/" << time << "_all.log";
    return {sstream_main.str(), sstream_mav.str()};
}

void initLogging(std::string directory, bool print_stderr, int argc, char* argv[]) {
    if (!print_stderr) {
        loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    }
    loguru::init(argc, argv);
    auto [readable_logs, everything_logs] = getLoggingFilenames(directory);
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
