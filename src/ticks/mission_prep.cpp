#include "ticks/mission_prep.hpp"

#include <memory>
#include <string>

#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/ids.hpp"

MissionPrepTick::MissionPrepTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionPrep) {}

std::chrono::milliseconds MissionPrepTick::getWait() const {
    return MISSION_PREP_TICK_WAIT;
}

Tick* MissionPrepTick::tick() {
    if (this->state->config.isValid()) {
        LOG_F(INFO, "Valid mission configuration detected");
        return new PathGenTick(this->state);
    } else {
        return nullptr;
    }
}
