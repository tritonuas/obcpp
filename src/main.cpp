
#include <chrono>
#include <string>

#include <loguru.hpp>

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

int main(int argc, char* argv[]) {
    initLogging(argc, argv);

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
