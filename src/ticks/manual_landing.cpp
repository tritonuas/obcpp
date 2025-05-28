#include "ticks/manual_landing.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

ManualLandingTick::ManualLandingTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    :Tick(state, TickID::ManualLanding), next_tick(next_tick) {}

std::chrono::milliseconds ManualLandingTick::getWait() const {
    return MANUAL_LANDING_TICK_WAIT;
}

Tick* ManualLandingTick::tick() {
    // TODO: Test this in mock testflight
    if (!state.get()->getMav()->isArmed()) {
        return next_tick;
    }
    return nullptr;
}
