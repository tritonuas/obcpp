#include "ticks/mission_prep.hpp"

#include <memory>
#include <string>

#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "ticks/path_gen.hpp"

MissionPreparationTick::MissionPreparationTick(std::shared_ptr<MissionState> state)
    :Tick(state) {}

std::chrono::milliseconds MissionPreparationTick::getWait() const {
    return MISSION_PREP_TICK_WAIT;
}

std::string MissionPreparationTick::getName() const {
    return "Mission Preparation";
}

Tick* MissionPreparationTick::tick() {
    if (this->state->config.isValid()) {
        LOG_F(INFO, "Valid mission configuration detected");
        return new PathGenerationTick(this->state);
    } else {
        return nullptr;
    }
}
