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
        // this->state->getCamera()->stopTakingPictures();
        // images = this->state->getCamera()->getAllImages();
        return new CVLoiterTick(this->state);
    }

    // TODO: Run Mavlink flight to find targets
    // Finish Loop
    std::pair<double, double> latlng = state->getMav()->latlng_deg();

    bool isAboveFlightzone = this->state->getMav()->isPointInPolygon(latlng, airdropBoundary);

    if (isAboveFlightzone  && ((getUnixTime_ms() - lastPhotoTime) > 1000ms)){
        
        auto photo = this->state->getCamera()->takePicture(1000ms, this->state->getMav());

        if (photo.has_value()){
            lastPhotoTime = getUnixTime_ms();
            this->state->getCV()->runPipeline(photo.value());
        }
    }

    //return nullptr;
}
