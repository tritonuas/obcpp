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

    for (int i = BottleDropIndex::A; i <= BottleDropIndex::E; i++) {
        if (dropped_bottles.contains(static_cast<BottleDropIndex>(i))) {
            continue;
        }

        next_bottle = static_cast<BottleDropIndex>(i);
        break;
    }
    state->markBottleAsDropped(next_bottle);

    auto cv_aggregator = state->getCV();
    {
        LockPtr<CVResults> results = cv_aggregator->getResults();
        auto target = results.data->detected_targets.at(results.data->matches.at(next_bottle).value_or(0));
        target.coord.set_altitude(state->config.pathing.approach.drop_altitude_m);

        LOG_F(INFO, "Routing to airdrop target %d at (%f, %f) alt %f", static_cast<int>(next_bottle),
            target.coord.latitude(), target.coord.longitude(), target.coord.altitude());

        state->setAirdropPath(generateAirdropApproach(state, target.coord));
    }

    state->getAirdrop()->send(makeArmPacket(
        DISARM, UDP2_ALL, OBC_NULL, state->getMav()->altitude_agl_m()));
        
    state->getAirdrop()->send(makeArmPacket(
        ARM, static_cast<bottle_t>(next_bottle), OBC_NULL, state->getMav()->altitude_agl_m()));

    return new MavUploadTick(this->state, new AirdropApproachTick(this->state),   
            state->getAirdropPath(), false);
}
