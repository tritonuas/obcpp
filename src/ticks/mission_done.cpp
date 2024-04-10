#include "ticks/mission_done.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

MissionDoneTick::MissionDoneTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionDone) {}

std::chrono::milliseconds MissionDoneTick::getWait() const {
    return MISSION_DONE_TICK_WAIT;
}

Tick* MissionDoneTick::tick() {
    return nullptr;
}
