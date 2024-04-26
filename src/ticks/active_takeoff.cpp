#include "ticks/active_takeoff.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"

ActiveTakeoffTick::ActiveTakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::Takeoff) {}

std::chrono::milliseconds ActiveTakeoffTick::getWait() const {
    return TAKEOFF_TICK_WAIT;
}

Tick* ActiveTakeoffTick::tick() {
    // TODO: figure out how to check mavsdk for flight mode, and if flight mode is
    // autonomous then go to next state
    
    return nullptr;
}
