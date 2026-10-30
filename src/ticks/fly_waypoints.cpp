#include "ticks/fly_waypoints.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "pathing/static.hpp"
#include "ticks/fly_search.hpp"
#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"
#include "utilities/common.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    : Tick(state, TickID::FlyWaypoints), mission_started(false), next_tick(next_tick),
      last_photo_time(0) {}

void FlyWaypointsTick::init() {
    while (!this->mission_started) {
        this->mission_started = this->state->getMav()->startMission();
        std::this_thread::sleep_for(50ms);
    }
    state->decrementLapsRemaining();

    LOG_F(INFO, "Started FlyWaypointsTick, Laps Remaining: %d", state->getLapsRemaining());
}

std::chrono::milliseconds FlyWaypointsTick::getWait() const { return FLY_WAYPOINTS_TICK_WAIT; }

Tick* FlyWaypointsTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr
    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (!isMissionFinished) {
        return nullptr;
    }

    if (state->getLapsRemaining() > 0) {
        return new MavUploadTick(
            this->state, new FlyWaypointsTick(this->state, new FlySearchTick(this->state)),
            state->getNextWaypointPath(), false);
    }

    return new MavUploadTick(
        this->state, new FlySearchTick(this->state),
        state->getCoveragePath(), false);
}
