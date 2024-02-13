#include "ticks/mission_start.hpp"

#include <memory>
#include <string>
#include <future>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mission_prep.hpp"
#include "network/mavlink.hpp"

MissionStartTick::MissionStartTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionStart) {}

std::chrono::milliseconds MissionStartTick::getWait() const {
    return MISSION_START_TICK_WAIT;
}

Tick* MissionStartTick::tick() {
    // TODO: figure out how to check mavsdk for flight mode, and if flight mode is
    // autonomous then go to next state
    return nullptr;
}
