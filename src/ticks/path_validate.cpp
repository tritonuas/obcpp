#include "ticks/path_validate.hpp"

#include <memory>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mission_upload.hpp"

PathValidateTick::PathValidateTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::PathValidate) {}

std::chrono::milliseconds PathValidateTick::getWait() const {
    return PATH_VALIDATE_TICK_WAIT;
}

Tick* PathValidateTick::tick() {
    if (this->state->isInitPathValidated() && this->state->getMav() != nullptr) {
        return new MissionUploadTick(this->state);
    } else {
        return nullptr;
    }
}
