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
    : Tick(state, TickID::AirdropApproach) {}

void AirdropApproachTick::init() {
    LOG_F(INFO, "start mission airdrop");
    this->state->getMav()->startMission();
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

Tick* AirdropApproachTick::tick() {
    if (state->getMav()->isAtFinalWaypoint()) {
        if (state->next_airdrop_to_drop.has_value()) {
            LOG_F(INFO, "Dropping airdrop %d", state->next_airdrop_to_drop.value());

            // Trigger the airdrop servo/relay
            triggerAirdrop(state->getMav(), state->next_airdrop_to_drop.value());

            // markAirdropAsDropped() has already been called in airdrop_prep, so don't update here
            // Lowkey kinda bad design but I didn't write this so don't blame me
            // If you want to uncomment the following lines to test if this shit works, make sure
            // to comment the ones in airdrop_prep

            // Convert airdrop_t to AirdropType when calling markAirdropAsDropped
            // state->markAirdropAsDropped(
                // static_cast<AirdropType>(state->next_airdrop_to_drop.value() - 1));

        } else {
            LOG_F(ERROR, "Cannot drop bottle because no bottle to drop");
        }
    }

    if (state->getMav()->isMissionFinished()) {
        if (state->getDroppedAirdrops().size() >= NUM_AIRDROPS) {
            return new ManualLandingTick(state, nullptr);
        } else {
            return new AirdropPrepTick(state);
        }
    }

    return nullptr;
}
