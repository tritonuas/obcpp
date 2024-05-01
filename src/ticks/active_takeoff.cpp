#include "ticks/active_takeoff.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"

ActiveTakeoffTick::ActiveTakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::ActiveTakeoff) {}

std::chrono::milliseconds ActiveTakeoffTick::getWait() const {
    return ACTIVE_TAKEOFF_TICK_WAIT;
}

void ActiveTakeoffTick::armAndHover() {
    this->takeoffResult = std::async(std::launch::async, [this](){ return this->state->getMav()->armAndHover(); });
}

Tick* ActiveTakeoffTick::tick() {
    auto takeoff = this->takeoffResult.wait_for(std::chrono::milliseconds(0));

    if(takeoff != std::future_status::ready) {
        return nullptr;
    }

    if(takeoff.second != true) {
        LOG_F(INFO, takeoff.first);
        return nullptr;
    }

    LOG_F(INFO, "Vehicle at required altitude");
    
    auto startMission = this->state->getMav()->startMission();

    if(startMIssion.second != true){
        LOG_F(INFO, startMission.first);
        return nullptr;
    }
            
    LOG_F(INFO, startMission.first);
    return new FlyWaypointsTick(this->state);
}
