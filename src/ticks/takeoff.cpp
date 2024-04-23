#include "ticks/takeoff.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"

TakeoffTick::TakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::Takeoff) {}

std::chrono::milliseconds TakeoffTick::getWait() const {
    return TAKEOFF_TICK_WAIT;
}

Tick* TakeoffTick::tick() {
    // TODO: figure out how to check mavsdk for flight mode, and if flight mode is
    // autonomous then go to next state
    if(state->getMav()->flight_mode() == mavsdk::Telemetry::FlightMode::Mission){
        LOG_F(INFO, "Plane has entered autonomous");
        return new FlyWaypointsTick(this->state);
    }
    
    return nullptr;
}
