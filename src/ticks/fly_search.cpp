#include "ticks/fly_search.hpp"

#include <memory>
#include <chrono>

#include "ticks/ids.hpp"
#include "utilities/common.hpp"
#include "ticks/cv_loiter.hpp"

using namespace std::chrono_literals; // NOLINT

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state):
    Tick(state, TickID::FlySearch)
{
    this->mission_started = false;
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

    auto latlng = state->getMav()->latlng_deg();
    bool isAboveFlightzone =
        this->state->getMav()->isPointInPolygon(latlng, this->airdrop_boundary);

    // Checks if the MAV is above the flight zone and
    // if the time since the last photo is greater than 1 second
    if (isAboveFlightzone  && ((getUnixTime_ms() - this->last_photo_time) > 1000ms)) {
        auto photo = this->state->getCamera()->takePicture(1000ms, this->state->getMav());

        if (photo.has_value()) {
            this->last_photo_time = getUnixTime_ms();            // Update the last photo time
            this->state->getCV()->runPipeline(photo.value());    // Run the pipeline on the photo
        }
    }

    return nullptr;
}
