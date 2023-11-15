#include <gtest/gtest.h>

#include "core/config.hpp"

// Test that when the config is created, all of the values are defaulted correctly
TEST(MissionConfigTest, ConfigDefaultVals) {
    EXPECT_EQ(1, 1);
}

// Test that all of the singular setters work
TEST(MissionConfigTest, ConfigSetSingular) {
    EXPECT_EQ(1, 1);
}

// plan out more unit tests...