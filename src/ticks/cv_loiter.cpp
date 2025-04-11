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
        const std::array<AirdropIndex, NUM_AIRDROPS> ALL_AIRDROPS = {
            AirdropIndex::Kaz, AirdropIndex::Kimi, AirdropIndex::Chris,
            AirdropIndex::Daniel
        };

        LockPtr<CVResults> results = state->getCV()->getResults();

        // for (const auto& bottle : ALL_BOTTLES) {
        //     // contains will never be false but whatever
        //     if (results.data->matches.contains(bottle)) {
        //         auto opt = results.data->matches.at(bottle);
        //         if (!opt.has_value()) {
        //             continue;
        //         }

        //         std::size_t index = opt.value();

        //         if (index >= results.data->detected_targets.size()) {
        //             continue;
        //         }

        //         auto target = results.data->detected_targets.at(index);

        //         float alt = state->getMav()->altitude_agl_m();

        //         LOG_F(INFO, "Sending coord(%f, %f) alt %f to bottle %d",
        //             target.coord.latitude(),
        //             target.coord.longitude(),
        //             alt,
        //             static_cast<int>(bottle));
        //         // assumes that the bottle_t enum in the udp2 stuff is the same as
        //         // BottleDropIndex enum
        //         state->getAirdrop()->send(makeLatLngPacket(
        //             SEND_LATLNG, static_cast<bottle_t>(bottle), OBC_NULL,
        //             target.coord.latitude(), target.coord.longitude(), alt));
        //     }
        // }

        return new AirdropPrepTick(this->state);
    } else if (status == Status::Rejected) {
        // TODO: Tell Mav to restart Search Mission
        return new FlySearchTick(this->state);
    }

    return nullptr;
}
