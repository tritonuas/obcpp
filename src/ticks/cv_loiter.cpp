#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {}

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

Tick* CVLoiterTick::tick() {
    return nullptr;
}
