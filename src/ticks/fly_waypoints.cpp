#include "ticks/fly_waypoints.hpp"

#include <chrono>
#include <future>
#include <memory>

#include "pathing/static.hpp"
#include "ticks/fly_search.hpp"
#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    : Tick(state, TickID::FlyWaypoints), next_tick(next_tick) {}

void FlyWaypointsTick::init() {
    // note: I didn't get around to testing if 1 would be a better value than 0
    // to see if the mission start can be forced.
    if (!this->state->getMav()->setMissionItem(1)) {
        LOG_F(ERROR, "Failed to reset Mission");
    }

    this->mission_started = this->state->getMav()->startMission();

    // I have another one here because idk how startmIssion behaves exactly
    if (!this->state->getMav()->setMissionItem(1)) {
        LOG_F(ERROR, "Failed to reset Mission");
    }
}

std::chrono::milliseconds FlyWaypointsTick::getWait() const { return FLY_WAYPOINTS_TICK_WAIT; }

Tick* FlyWaypointsTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr
    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (!isMissionFinished) {
        return nullptr;
    }

    state->config.pathing.laps--;
    if (state->config.pathing.laps > 0) {
        // regenerate path
        std::future<MissionPath> init_path;
        init_path = std::async(std::launch::async, generateInitialPath, this->state);
        auto init_status = init_path.wait_for(std::chrono::milliseconds(0));
        int count_ms = 2500;
        const int wait_time_ms = 100;

        while (init_status != std::future_status::ready && count_ms > 0) {
            LOG_F(WARNING, "Waiting for path to be generated...");
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
            init_status = init_path.wait_for(std::chrono::milliseconds(0));
            count_ms -= wait_time_ms;
        }

        if (count_ms < 0) {
            LOG_F(ERROR, "Path generation took too long. Trying Again...");
            return nullptr;
        }

        state->setInitPath(init_path.get());

        return new MavUploadTick(
            this->state, new FlyWaypointsTick(this->state, new FlySearchTick(this->state)),
            state->getInitPath(), false);
    }

    return new MavUploadTick(
        this->state, new FlySearchTick(this->state),
        state->getCoveragePath(), false);
}
