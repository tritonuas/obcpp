#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_search.hpp"


CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {
        status = CVLoiterTick::Status::None;
    }

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

void CVLoiterTick::setStatus(Status status) {
    this->status = status;
}

Tick* CVLoiterTick::tick() {
    //Tick is called if Search Zone coverage path is finished

    // Check status of the CV Results
    if (status == Status::Validated) {
        return new AirdropPrepTick(this->state);
    } 

    // If not all targets are validated invoke Flysearch again to attept to locate the target
    else if (status == Status::Rejected) {
        // TODO: Tell Mav to restart Search Mission
        return new FlySearchTick(this->state);
    }

    return nullptr;
}
