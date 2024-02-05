#include "ticks/takeoff_prep.hpp"

#include <memory>
#include <string>

#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"

TakeoffPrepTick::TakeoffPrepTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::TakeoffPrep) {}

std::chrono::milliseconds TakeoffPrepTick::getWait() const {
    return TAKEOFF_PREP_TICK_WAIT;
}

Tick* TakeoffPrepTick::tick() {
    // TODO: upload path to plane
    return nullptr;  // TODO: figure out how to transition past this
}
