#include "ticks/active_takeoff.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"

ActiveTakeoffTick::ActiveTakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::Takeoff) {}

std::chrono::milliseconds ActiveTakeoffTick::getWait() const {
    return ACTIVE_TAKEOFF_TICK_WAIT;
}

Tick* ActiveTakeoffTick::tick() {
    std::future <std::pair <std::string, bool> > takeoffResult = std::async(std::launch::async, [this](){
        return this->state->getMav()->armAndHover();
    });
    
    std::pair <std::string, bool> result = takeoffResult.get();

    if(result.second == true){
        LOG_F(INFO, "Plane has finished take off.");
        return new FlyWaypointsTick(this->state);
    }

    LOG_F(INFO, result.first);
    return nullptr;
}
