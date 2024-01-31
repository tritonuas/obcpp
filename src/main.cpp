#include "core/obc.hpp"
#include "utilities/constants.hpp"

#include <loguru.hpp>

int main() {
    LOG_F(INFO, "Starting OBC...");
    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
