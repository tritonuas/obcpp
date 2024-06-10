#include "ticks/wait_for_takeoff.hpp"

#include <memory>
#include <future>

#include "ticks/takeoff.hpp"
#include "ticks/active_takeoff.hpp"

WaitForTakeoffTick::WaitForTakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::WaitForTakeoff), status(WaitForTakeoffTick::Status::None) {
}

std::chrono::milliseconds WaitForTakeoffTick::getWait() const {
    return WAIT_FOR_TAKEOFF_TICK_WAIT;
}

Tick* WaitForTakeoffTick::tick() {
    if (status == Status::Manual) {
        LOG_F(INFO, "Transitioning to manual takeoff tick.");
        return new TakeoffTick(this->state);
    } else if (status == Status::Autonomous) {
        LOG_F(INFO, "Transitioning to autonomous takeoff tick.");
        return new ActiveTakeoffTick(this->state);
    }
    return nullptr;
}

void WaitForTakeoffTick::setStatus(WaitForTakeoffTick::Status status) {
    this->status = status;
}
