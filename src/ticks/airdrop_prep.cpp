#include "ticks/airdrop_prep.hpp"

#include <memory>
#include <string>

#include "core/mission_state.hpp"
#include "pathing/static.hpp"
#include "ticks/airdrop_approach.hpp"
#include "ticks/ids.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/manual_landing.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/logging.hpp"

AirdropPrepTick::AirdropPrepTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::AirdropPrep) {}

std::chrono::milliseconds AirdropPrepTick::getWait() const { return AIRDROP_PREP_TICK_WAIT; }

Tick* AirdropPrepTick::tick() {
    BottleDropIndex next_bottle = BottleDropIndex::A;

    auto dropped_bottles = state->getDroppedBottles();

    if (dropped_bottles.size() >= NUM_AIRDROP_BOTTLES) {
        return new ManualLandingTick(state);
    }

    LockPtr<CVResults> results = state->getCV()->getResults();

    for (int i = BottleDropIndex::A; i <= BottleDropIndex::E; i++) {
        if (dropped_bottles.contains(static_cast<BottleDropIndex>(i))) {
            continue;
        }

        next_bottle = static_cast<BottleDropIndex>(i);

        if (!results.data->matches.at(next_bottle).has_value()) {
            LOG_F(INFO, "Skipping bottle %d because we didn't match it",
                static_cast<int>(next_bottle));
            state->markBottleAsDropped(next_bottle);
            continue;
        }

        break;
    }
    state->markBottleAsDropped(next_bottle);

    // The or condition here shouldn't be met because above we check for value
    // before setting next_bottle.
    // But just in case we default to whatever location target 0 was found at.
    auto target = results.data->detected_targets.at(
        results.data->matches.at(next_bottle).value_or(0));
    // IMPORTANT: need to set the altitude of the target coord to the config value so it doesn't
    // try and nosedive into the ground...
    target.coord.set_altitude(state->config.pathing.approach.drop_altitude_m);

    LOG_F(INFO, "Routing to airdrop target %d at (%f, %f) alt %f", static_cast<int>(next_bottle),
        target.coord.latitude(), target.coord.longitude(), target.coord.altitude());

    state->setAirdropPath(generateAirdropApproach(state, target.coord));

    state->next_bottle_to_drop = static_cast<bottle_t>(next_bottle);

    // If we ever switch to use the actual guided part of the protocol probably want
    // to uncomment these out and change where currently we are sending the do drop now command

    // state->getAirdrop()->send(makeArmPacket(
    //     DISARM, UDP2_ALL, OBC_NULL, state->getMav()->altitude_agl_m()));

    // state->getAirdrop()->send(makeArmPacket(
    //     ARM, static_cast<bottle_t>(next_bottle), OBC_NULL, state->getMav()->altitude_agl_m()));

    return new MavUploadTick(this->state, new AirdropApproachTick(this->state),
            state->getAirdropPath(), false);
}
