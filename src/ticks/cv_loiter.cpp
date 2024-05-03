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
    //Tick is called if Search Zone coverage path is finished

    //Check if all expected targets are found 
    if (false) {
        return new AirdropPrepTick(this->state);
    } 
    // If not all targets are validated invoke Flysearch again to attept to locate the target
    else if (false) {
        return new FlySearchTick(this->state);
    }
    return nullptr;
}
