#include "ticks/fly_search.hpp"

#include <memory>
#include <chrono>

#include "ticks/ids.hpp"
#include "utilities/common.hpp"
#include "ticks/cv_loiter.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state):
    Tick(state, TickID::FlySearch) {
    this->mission_started = false;
    this->curr_mission_item = 1;  // if this was 0 it would take a picture immediately after
    // entering the search mission, so set to 1 so it doesn't start taking pictures until
    // actually over the search zone
}

std::chrono::milliseconds FlySearchTick::getWait() const {
    return FLY_SEARCH_TICK_WAIT;
}

void FlySearchTick::init() {
    this->state->getCamera()->startStreaming();
    this->airdrop_boundary = this->state->mission_params.getAirdropBoundary();
    this->last_photo_time = getUnixTime_ms();

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

    LOG_F(INFO, "Total Waypoint #: %zu", this->state->getMav()->totalWaypoints());
}


Tick* FlySearchTick::tick() {
    if (!this->mission_started) {
        this->mission_started = this->state->getMav()->startMission();
        return nullptr;
    }

    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (isMissionFinished) {
        // Default MAV Behavior is to Loiter after finishing the mission,
        // so we can just return a CVLoiterTick
        return new CVLoiterTick(this->state);
    }
    return nullptr;
}
