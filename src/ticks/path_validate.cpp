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
    if (!this->state->recvTickMsg<PathValidateTick>(status)) {
        return nullptr;
    }

    switch (status) {
        case Message::Rejected:
            return new PathGenTick(this->state);
        case Message::Validated:
            if (this->state->getMav() != nullptr) {
                return new MissionUploadTick(this->state);
            }
    }

    return nullptr;
}
