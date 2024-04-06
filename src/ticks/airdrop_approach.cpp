#include "ticks/airdrop_approach.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

AirdropApproachTick::AirdropApproachTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::AirdropApproach) {}

std::chrono::milliseconds AirdropApproachTick::getWait() const {
    return AIRDROP_APPROACH_TICK_WAIT;
}

Tick* AirdropApproachTick::tick() {
    return nullptr;
}
