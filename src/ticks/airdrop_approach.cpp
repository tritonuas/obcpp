#include "ticks/airdrop_approach.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/airdrop_prep.hpp"
#include "utilities/constants.hpp"

AirdropApproachTick::AirdropApproachTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::AirdropApproach) {}

void AirdropApproachTick::init() {
    state->getMav()->startMission();
}

std::chrono::milliseconds AirdropApproachTick::getWait() const {
    return AIRDROP_APPROACH_TICK_WAIT;
}

Tick* AirdropApproachTick::tick() {
    if (state->getMav()->isMissionFinished()) {
        return new MavUploadTick(state, new FlyWaypointsTick(state,
            new AirdropPrepTick(state)), state->getInitPath(), false);
    }

    return nullptr;
}
