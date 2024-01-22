#include "core/obc.hpp"
#include "utilities/constants.hpp"

int main() {
    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
