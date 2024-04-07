#include "ticks/airdrop_prep.hpp"

#include <memory>
#include <string>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/ids.hpp"

AirdropPrepTick::AirdropPrepTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::AirdropPrep) {}

std::chrono::milliseconds AirdropPrepTick::getWait() const {
    return AIRDROP_PREP_TICK_WAIT;
}

Tick* AirdropPrepTick::tick() {
    return nullptr;
}
