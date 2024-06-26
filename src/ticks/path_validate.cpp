#include "ticks/path_validate.hpp"

#include <memory>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/takeoff.hpp"
#include "ticks/wait_for_takeoff.hpp"
#include "utilities/locks.hpp"

PathValidateTick::PathValidateTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::PathValidate), status(PathValidateTick::Status::None) {}

std::chrono::milliseconds PathValidateTick::getWait() const {
    return PATH_VALIDATE_TICK_WAIT;
}

Tick* PathValidateTick::tick() {
    if (status == Status::Validated) {
        if (this->state->getMav() != nullptr) {
            return new MavUploadTick(this->state, new WaitForTakeoffTick(this->state),
                state->getInitPath(), true);
        } else {
            LOG_F(WARNING, "Path Validated, but cannot continue because mavlink not connected.");
            return nullptr;
        }
    } else if (status == Status::Rejected) {
        return new PathGenTick(this->state);
    }

    return nullptr;
}

void PathValidateTick::setStatus(PathValidateTick::Status status) {
    this->status = status;
}
