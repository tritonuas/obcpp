#include "ticks/fly_search.hpp"

#include <memory>
#include <chrono>

#include "ticks/ids.hpp"
#include "utilities/common.hpp"
#include "ticks/cv_loiter.hpp"

using namespace std::chrono_literals;

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlySearch) {
    //     const std::chrono::milliseconds interval {2000}; // Might need tune
        this->state->getCamera()->startStreaming();
        airdropBoundary = this->state->mission_params.getAirdropBoundary();
        lastPhotoTime = getUnixTime_ms();
    }

std::chrono::milliseconds FlySearchTick::getWait() const {
    return FLY_SEARCH_TICK_WAIT;
}

Tick* FlySearchTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr

    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (isMissionFinished) {
        // Default MAV Behavior is to Loiter after finishing the mission, so we can just return a CVLoiterTick
        return new CVLoiterTick(this->state);
    }

    // TODO: Should replace these pairs with GPSCords
    std::pair<double, double> latlng = state->getMav()->latlng_deg();
    bool isAboveFlightzone = this->state->getMav()->isPointInPolygon(latlng, airdropBoundary);

    // Checks if the MAV is above the flight zone and if the time since the last photo is greater than 1 second
    if (isAboveFlightzone  && ((getUnixTime_ms() - lastPhotoTime) > 1000ms)){
        auto photo = this->state->getCamera()->takePicture(1000ms, this->state->getMav());

        if (photo.has_value()){
            lastPhotoTime = getUnixTime_ms();                    // Update the last photo time
            this->state->getCV()->runPipeline(photo.value());    // Run the pipeline on the photo
        }
    }

    return nullptr;
}
