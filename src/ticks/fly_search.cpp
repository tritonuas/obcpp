#include "ticks/fly_search.hpp"

#include <memory>
#include <chrono>

#include "ticks/ids.hpp"
#include "utilities/common.hpp"
#include "ticks/cv_loiter.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state):
    Tick(state, TickID::FlySearch)
{
    this->mission_started = false;
    this->curr_mission_item = 1; // if this was 0 it would take a picture immediately after
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

    this->mission_started = this->state->getMav()->startMission();
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

    // IMPORTANT: currently hardcoded to assume hover search pathing, so it
    // takes photos whenever it gets to a new waypoint (loiter position)
    // if we were doing forward pathing would probably want to make it 
    // take photos at an interval but only when over the zone
    auto curr_waypoint = this->state->getMav()->curr_waypoint();
    if (this->curr_mission_item != curr_waypoint) {
        for (int i = 0; i < this->state->config.pathing.coverage.hover.pictures_per_stop; i++) {
            auto photo = this->state->getCamera()->takePicture(500ms, this->state->getMav());

            if (photo.has_value()) {
                this->last_photo_time = getUnixTime_ms();            // Update the last photo time
                this->state->getCV()->runPipeline(photo.value());    // Run the pipeline on the photo
            }
        }
        this->curr_mission_item = curr_waypoint;

        return nullptr;
    }

    return nullptr;
}
