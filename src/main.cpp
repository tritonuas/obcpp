#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

#include <loguru.hpp>

#include <chrono>
#include <string>

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);
    loguru::add_file(getLoggingFilename(argc, argv).c_str(),
        loguru::Truncate, loguru::Verbosity_MAX);

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
