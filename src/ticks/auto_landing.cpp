#include "ticks/auto_landing.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

AutoLandingTick::AutoLandingTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::AutoLanding) {}

std::chrono::milliseconds AutoLandingTick::getWait() const {
    return AUTO_LANDING_TICK_WAIT;
}

Tick* AutoLandingTick::tick() {
    return nullptr;
}
