#include "ticks/fly_search.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlySearch) {
        const std::chrono::milliseconds interval {2000}; // Might need tune
        this->state->getCamera()->startTakingPictures(interval, this->state->getMav());
    }

std::chrono::milliseconds FlySearchTick::getWait() const {
    return FLY_SEARCH_TICK_WAIT;
}

Tick* FlySearchTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr

    // TODO: Run Mavlink flight to find targets
    // Finish Loop

    // Deconstructor
    // TODO: Detect when path is completed
    if (false) {
        this->state->getCamera()->stopTakingPictures();
        // images = this->state->getCamera()->getAllImages();
        return new CVLoiterTick(this->state);
    }
    //return nullptr;
}
