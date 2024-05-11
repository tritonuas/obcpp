#include "ticks/active_takeoff.hpp"

#include <memory>
#include <future>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/mission_prep.hpp"

ActiveTakeoffTick::ActiveTakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::ActiveTakeoff) {
    this->started = false;
}

std::chrono::milliseconds ActiveTakeoffTick::getWait() const {
    return ACTIVE_TAKEOFF_TICK_WAIT;
}

void ActiveTakeoffTick::armAndHover() {
    this->takeoffResult = std::async(std::launch::async,
        [this](){ return this->state->getMav()->armAndHover(this->state); });
}

Tick* ActiveTakeoffTick::tick() {
    if (!started) {
        started = true;
        this->armAndHover();
        return nullptr;
    }

    auto takeoff = this->takeoffResult.wait_for(std::chrono::milliseconds(0));

    if (takeoff != std::future_status::ready) {
        return nullptr;
    }

    auto result = takeoffResult.get();

    if (result != true) {
        return new MissionPrepTick(this->state);
    }

    LOG_F(INFO, "Vehicle at required altitude");

    auto startMission = this->state->getMav()->startMission();

    if (startMission != true) {
        return new MissionPrepTick(this->state);
    }

    return new FlyWaypointsTick(this->state);
}
