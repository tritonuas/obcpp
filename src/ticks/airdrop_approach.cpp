#include "ticks/airdrop_approach.hpp"

#include <memory>

#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/ids.hpp"
#include "ticks/manual_landing.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/refueling.hpp"
#include "ticks/wait_for_takeoff.hpp"
#include "utilities/constants.hpp"

AirdropApproachTick::AirdropApproachTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::AirdropApproach) {}

void AirdropApproachTick::init() { state->getMav()->startMission(); }

std::chrono::milliseconds AirdropApproachTick::getWait() const {
    return AIRDROP_APPROACH_TICK_WAIT;
}

Tick* AirdropApproachTick::tick() {
    if (state->getMav()->isAtFinalWaypoint()) {
        if (state->next_bottle_to_drop.has_value()) {
            LOG_F(INFO, "Dropping bottle %d", state->next_bottle_to_drop.value());
            state->getAirdrop()->send(makeDropNowPacket(state->next_bottle_to_drop.value()));
            state->getAirdrop()->send(makeDropNowPacket(state->next_bottle_to_drop.value()));
            state->getAirdrop()->send(makeDropNowPacket(state->next_bottle_to_drop.value()));
        } else {
            LOG_F(ERROR, "Cannot drop bottle because no bottle to drop");
        }
    }

    if (state->getMav()->isMissionFinished()) {
        if (state->getDroppedBottles().size() >= NUM_AIRDROP_BOTTLES) {
            return new ManualLandingTick(state, nullptr);
        } else if (state->getDroppedBottles().size() == 2) {
            return new ManualLandingTick(state, new RefuelingTick(state));
        } else {
            return new MavUploadTick(state, new FlyWaypointsTick(state, new AirdropPrepTick(state)),
                                     state->getInitPath(), false);
        }
    }

    return nullptr;
}
