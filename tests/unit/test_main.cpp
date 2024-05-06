#include <gtest/gtest.h>

#include <iostream>

#include "utilities/logging.hpp"

using ::testing::InitGoogleTest;

int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    // todo pull from config
    initLogging("/workspaces/obcpp/tests/unit/logs", true, argc, argv);
    return RUN_ALL_TESTS();
}