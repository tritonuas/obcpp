#include "ticks/manual_landing.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

ManualLandingTick::ManualLandingTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    : Tick(state, TickID::ManualLanding), next_tick(next_tick) {


    }

std::chrono::milliseconds ManualLandingTick::getWait() const { return MANUAL_LANDING_TICK_WAIT; }

Tick* ManualLandingTick::tick() {
    // I couldn't figure out if it can get armed/disarmed rn so it just returns
    return next_tick;
}
