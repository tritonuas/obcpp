#include "ticks/fly_waypoints.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlyWaypoints) {}

std::chrono::milliseconds FlyWaypointsTick::getWait() const {
    return FLY_WAYPOINTS_TICK_WAIT;
}

Tick* FlyWaypointsTick::tick() {
    return nullptr;
}
