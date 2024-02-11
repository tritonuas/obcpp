#include <mavsdk/log_callback.h>

#include <chrono>
#include <string>

#include <loguru.hpp>

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);
    loguru::add_file(getLoggingFilename(argc, argv).c_str(),
        loguru::Truncate, loguru::Verbosity_MAX);

    // Overwrite default mavsdk logging to standard out,
    // and redirect to loguru
    mavsdk::log::subscribe(
        [](mavsdk::log::Level level, const std::string& message,
           const std::string& file, int line) {
        std::string parsed_msg = "MavSDK@" + file + ":" + std::to_string(line) + "// " + message;
        switch (level) {
            case mavsdk::log::Level::Debug:
                // Do nothing
                break;
            case mavsdk::log::Level::Info:
                LOG_F(INFO, "%s", parsed_msg.c_str());
                break;
            case mavsdk::log::Level::Warn:
                LOG_F(WARNING, "%s", parsed_msg.c_str());
                break;
            case mavsdk::log::Level::Err:
                LOG_F(ERROR, "%s", parsed_msg.c_str());
                break;
        }

        return true;  // never log to standard out
    });

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
