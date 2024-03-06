#include "ticks/path_validate.hpp"

#include <memory>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mission_upload.hpp"
#include "ticks/path_gen.hpp"
#include "utilities/locks.hpp"

PathValidateTick::PathValidateTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::PathValidate) {}

std::chrono::milliseconds PathValidateTick::getWait() const {
    return PATH_VALIDATE_TICK_WAIT;
}

Tick* PathValidateTick::tick() {
    Message status;
    if (!this->already_validated && !this->state->recvTickMsg<PathValidateTick>(&status)) {
        // haven't already received a validated msg & there is no current msg
        return nullptr;
    }

    if (this->already_validated || status == Message::Validated) {
        if (this->state->getMav() != nullptr) {
            return new MissionUploadTick(this->state);
        } else {
            this->already_validated = true;
            LOG_F(WARNING, "Path Validated, but cannot continue because mavlink not connected.");
            return nullptr;
        }
    } else if (status == Message::Rejected) {
        return new PathGenTick(this->state);
    } else {
        LOG_F(ERROR, "Invalid id %d passed into PathValidateTick::tick()",
            static_cast<int>(status));
    }

    return nullptr;
}
