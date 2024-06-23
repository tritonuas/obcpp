#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_search.hpp"


CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state):
    Tick(state, TickID::CVLoiter) {
    this->status = CVLoiterTick::Status::None;
}

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

void CVLoiterTick::setStatus(Status status) {
    this->status = status;
}

Tick* CVLoiterTick::tick() {
    // Tick is called if Search Zone coverage path is finished

    // // print out current state of matching for debugging
    // LockPtr<CVResults> cv_results = this->state->getCV()->getResults();
    // for (const auto& match: cv_results.data->matches) {
    //     if (match.second.has_value()) {
    //         LOG_F(INFO, "Bottle %d is matched with target at lat: %f, lon: %f",
    //             match.first,
    //             cv_results.data->detected_targets.at(match.second.value()).coord.latitude(),
    //             cv_results.data->detected_targets.at(match.second.value()).coord.longitude()
    //         );
    //     } else {
    //         LOG_F(INFO, "Bottle %d is not matched with a target", match.first);
    //     }
    // }

    // Check status of the CV Results
    if (status == Status::Validated) {
        return new AirdropPrepTick(this->state);
    } else if (status == Status::Rejected) {
        // TODO: Tell Mav to restart Search Mission
        return new FlySearchTick(this->state);
    }

    return nullptr;
}
