#include "ticks/airdrop_approach.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/manual_landing.hpp"
#include "utilities/constants.hpp"
#include "ticks/refueling.hpp"
#include "ticks/wait_for_takeoff.hpp"

AirdropApproachTick::AirdropApproachTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::AirdropApproach) {}

void AirdropApproachTick::init() {
    LOG_F(INFO, "start mission airdrop");
    this->state->getMav()->startMission();
}

std::chrono::milliseconds AirdropApproachTick::getWait() const {
    return AIRDROP_APPROACH_TICK_WAIT;
}

Tick* AirdropApproachTick::tick() {
    if (state->getMav()->isAtFinalWaypoint()) {
        if (state->next_airdrop_to_drop.has_value()) {
            LOG_F(INFO, "Dropping airdrop %d", state->next_airdrop_to_drop.value());
            // state->getAirdrop()->send(makeDropNowPacket(state->next_airdrop_to_drop.value()));
            // state->getAirdrop()->send(makeDropNowPacket(state->next_airdrop_to_drop.value()));
            // state->getAirdrop()->send(makeDropNowPacket(state->next_airdrop_to_drop.value()));
        } else {
            LOG_F(ERROR, "Cannot drop bottle because no bottle to drop");
        }
    }

    if (state->getMav()->isMissionFinished()) {
        if (state->getDroppedAirdrops().size() >= NUM_AIRDROPS) {
            return new ManualLandingTick(state, nullptr);
        } else if (state->getDroppedAirdrops().size() % state->config.pathing.approach.payload_size == 0) {
            return new ManualLandingTick(state, new RefuelingTick(state));
        } else {
            return new MavUploadTick(state, new FlyWaypointsTick(state,
                new AirdropPrepTick(state)), state->getInitPath(), false);
        }
    }

    return nullptr;
}
