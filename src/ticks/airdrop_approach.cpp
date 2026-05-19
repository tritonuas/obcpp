#include "ticks/airdrop_approach.hpp"

#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

#include <chrono>
#include <memory>
#include <thread>

#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/ids.hpp"
#include "ticks/manual_landing.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"
#include "ticks/refueling.hpp"
#include "ticks/wait_for_takeoff.hpp"


AirdropApproachTick::AirdropApproachTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::AirdropApproach), airdrop_triggered(false) {}

void AirdropApproachTick::init() {
    LOG_F(INFO, "start mission airdrop");

    bool mission_reset = false;
    while (!mission_reset) {
        mission_reset = this->state->getMav()->setMissionItem(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    bool mission_started = false;
    while (!mission_started) {
        mission_started = this->state->getMav()->startMission();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::chrono::milliseconds AirdropApproachTick::getWait() const {
    return AIRDROP_APPROACH_TICK_WAIT;
}

// Helper function to trigger the airdrop mechanism
bool triggerAirdrop(std::shared_ptr<MavlinkClient> mav, airdrop_t airdrop_index) {
    LOG_F(INFO, "Triggering airdrop mechanism for airdrop %d", static_cast<int>(airdrop_index));

    // Use the new triggerRelay method to activate RELAY2 (relay index 1)
    // For ArduPilot, RELAY2 corresponds to relay_number = 1 (zero-indexed)
    bool success = mav->triggerRelay(1, true);

    if (success) {
        LOG_F(INFO, "Successfully activated RELAY2 for airdrop");

        // Sleep briefly to ensure the relay has time to activate
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Optional: Turn off the relay after a delay
        // This depends on your specific hardware setup - some relays may need to be turned off
        // after activation, while others might reset automatically
        if (mav->triggerRelay(1, false)) {
            LOG_F(INFO, "Successfully deactivated RELAY2 after airdrop");
        } else {
            LOG_F(WARNING, "Failed to deactivate RELAY2 after airdrop");
        }
    } else {
        LOG_F(ERROR, "Failed to activate RELAY2 for airdrop");
    }

    return success;
}

void AirdropApproachTick::dropAirdrop() {
    airdrop_t next_airdrop = state->next_airdrop_to_drop.value();
    LOG_F(INFO, "Dropping airdrop %d", next_airdrop);
    
    this->airdrop_triggered = true;
    state->markAirdropAsDropped(static_cast<AirdropType>(next_airdrop));
    state->next_airdrop_to_drop.reset();
}

Tick* AirdropApproachTick::tick() {
    bool at_final_waypoint = state->getMav()->isAtFinalWaypoint();
    bool mission_finished = state->getMav()->isMissionFinished();

    if (!this->airdrop_triggered && (at_final_waypoint || mission_finished)) {
        if (mission_finished && !at_final_waypoint) {
            LOG_F(WARNING, "Airdrop approach mission finished before final waypoint tick");
        }
        this->dropAirdrop();
    }

    if (mission_finished) {
        if (!this->airdrop_triggered) {
            LOG_F(WARNING, "Airdrop approach mission finished before airdrop was triggered");
            return nullptr;
        }

        if (state->getDroppedAirdrops().size() >= NUM_AIRDROPS) {
            return new ManualLandingTick(state, nullptr);
        } else {
            return new AirdropPrepTick(state);
        }
    }

    return nullptr;
}
