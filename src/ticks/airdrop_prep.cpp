#include "ticks/airdrop_prep.hpp"

#include <memory>
#include <string>

#include "core/mission_state.hpp"
#include "pathing/static.hpp"
#include "ticks/airdrop_approach.hpp"
#include "ticks/ids.hpp"
#include "ticks/manual_landing.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/path_gen.hpp"
#include "utilities/logging.hpp"

AirdropPrepTick::AirdropPrepTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::AirdropPrep) {}

std::chrono::milliseconds AirdropPrepTick::getWait() const { return AIRDROP_PREP_TICK_WAIT; }

Tick* AirdropPrepTick::tick() {
    AirdropIndex next_airdrop = AirdropIndex::Kaz;

    auto dropped_airdrops = state->getDroppedAirdrops();

    if (dropped_airdrops.size() >= NUM_AIRDROPS) {
        return new ManualLandingTick(state, nullptr);
    }

    LockPtr<MatchedResults> results = state->getCV()->getMatchedResults();

    for (int i = AirdropIndex::Kaz; i <= AirdropIndex::Daniel; i++) {
        if (dropped_airdrops.contains(static_cast<AirdropIndex>(i))) {
            continue;
        }

        next_airdrop = static_cast<AirdropIndex>(i);

        // Dont think we need this, as everything theoretically should be matched when
        // the matched route is posted.
        // if (!results.data->matches.at(next_airdrop).has_value()) {
        //     LOG_F(INFO, "Skipping bottle %d because we didn't match it",
        //           static_cast<int>(next_bottle));
        //     state->markAirdropAsDropped(next_bottle);
        //     continue;
        // }

        break;
    }
    state->markAirdropAsDropped(next_airdrop);

    // The or condition here shouldn't be met because above we check for value
    // before setting next_bottle.
    // But just in case we default to whatever location target 0 was found at.
    // auto target =
        // results.data->detected_targets.at(results.data->matches.at(next_airdrop).value_or(0));
    auto target = results.data->matched_airdrop.at(next_airdrop);
    // IMPORTANT: need to set the altitude of the target coord to the config value so it doesn't
    // try and nosedive into the ground...
    target.mutable_coordinate()->set_altitude(state->config.pathing.approach.drop_altitude_m);
    
    // Use for testing cuz testing images don't have proper GPS coordinates attached to them
    // Uncomment the following two lines for sitl testing
    // target.mutable_coordinate()->set_latitude(38.31584541086285);
    // target.mutable_coordinate()->set_longitude(-76.55316855169548);

    LOG_F(INFO, "Routing to airdrop target %d at (%f, %f) alt %f", static_cast<int>(next_airdrop),
          target.coordinate().latitude(), target.coordinate().longitude(), target.coordinate().altitude());

    state->setAirdropPath(generateAirdropApproach(state, target.coordinate()));

    LOG_F(INFO, "Generated approach path");

    state->next_airdrop_to_drop = static_cast<airdrop_t>(next_airdrop);

    // If we ever switch to use the actual guided part of the protocol probably want
    // to uncomment these out and change where currently we are sending the do drop now command

    // state->getAirdrop()->send(makeArmPacket(
    //     DISARM, UDP2_ALL, OBC_NULL, state->getMav()->altitude_agl_m()));

    // state->getAirdrop()->send(makeArmPacket(
    //     ARM, static_cast<bottle_t>(next_bottle), OBC_NULL,
    // state->getMav()->altitude_agl_m()));

    return new MavUploadTick(this->state, new AirdropApproachTick(this->state),
                             state->getAirdropPath(), false);
}
