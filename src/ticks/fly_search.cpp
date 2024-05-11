#include "ticks/fly_search.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

FlySearchTick::FlySearchTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlySearch) {}

std::chrono::milliseconds FlySearchTick::getWait() const {
    return FLY_SEARCH_TICK_WAIT;
}

Tick* FlySearchTick::tick() {
    return nullptr;
}
