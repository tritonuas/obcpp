#include <gtest/gtest.h>
#include <memory>

#include "network/gcs.hpp"
#include "core/mission_state.hpp"

TEST(GCSServerTest, GetMissionNoMission) {
    std::shared_ptr<MissionState> state;
}