#include "ticks/airdrop_prep.hpp"

#include <memory>
#include <string>

#include "core/mission_state.hpp"
#include "pathing/static.hpp"
#include "ticks/ids.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/airdrop_approach.hpp"
#include "utilities/logging.hpp"

AirdropPrepTick::AirdropPrepTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::AirdropPrep) {}

std::chrono::milliseconds AirdropPrepTick::getWait() const { return AIRDROP_PREP_TICK_WAIT; }

Tick* AirdropPrepTick::tick() {
    std::shared_ptr<CVAggregator> cv_aggregator = state->getCV();
    LockPtr<CVResults> results = cv_aggregator->getResults();
    BottleDropIndex next_bottle = BottleDropIndex::A;

    for (int i = BottleDropIndex::A; i <= BottleDropIndex::E; i++) {
        if (state->dropped_bottles.contains((BottleDropIndex)i)) {
            continue;
        }

        next_bottle = (BottleDropIndex)i;
        break;
    }

    DetectedTarget target = results.ptr->detected_targets.at(results.ptr->matches.at(next_bottle));
    
    state->current_path = generateAirdropApproach(state, target.coord);

    return new AirdropApproachTick(state);
}
