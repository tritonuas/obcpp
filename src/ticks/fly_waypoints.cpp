#include "ticks/fly_waypoints.hpp"
#include "ticks/fly_search.hpp"
#include <memory>

#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"
#include "utilities/common.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    :Tick(state, TickID::FlyWaypoints), next_tick(next_tick) {}

void FlyWaypointsTick::init() {
    state->getMav()->startMission();
    state->getCamera()->startStreaming();
    this->last_photo_time = getUnixTime_ms();
}

std::chrono::milliseconds FlyWaypointsTick::getWait() const {
    return FLY_WAYPOINTS_TICK_WAIT;
}

Tick* FlyWaypointsTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr
    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (isMissionFinished) {
        if (state->config.pathing.laps > 1) {
            state->config.pathing.laps--;
            state->getMav()->startMission();
            return nullptr;
        }
        return next_tick;
    }

    return nullptr;
}
