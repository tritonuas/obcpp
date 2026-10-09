#include "ticks/fly_search.hpp"

#include <memory>
#include <chrono>
#include <thread>

#include "pathing/environment.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/cv_loiter.hpp"
#include "ticks/ids.hpp"
#include "utilities/common.hpp"

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

    while (!this->mission_started) {
        this->mission_started = this->state->getMav()->startMission();
        std::this_thread::sleep_for(50ms);
    }

    LOG_F(INFO, "Total Waypoint #: %zu", this->state->getMav()->totalWaypoints());
}


Tick* FlySearchTick::tick() {
    MissionState::CVStatus status = this->state->getCVStatus();
    if (status == MissionState::CVStatus::Validated) {
        this->state->setCVStatus(MissionState::CVStatus::None);
        return new AirdropPrepTick(this->state);
    }

    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (isMissionFinished) {
        // Default MAV Behavior is to Loiter after finishing the mission,
        // so we can just return a CVLoiterTick
        return new CVLoiterTick(this->state);
    }
    return nullptr;
}
