#include "ticks/mission_upload.hpp"

#include <memory>
#include <string>
#include <future>

#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/mission_start.hpp"
#include "network/mavlink.hpp"

MissionUploadTick::MissionUploadTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionUpload) {
    this->mission_uploaded = std::async(std::launch::async,
                                        &MavlinkClient::uploadMissionUntilSuccess,
                                        this->state->getMav(),
                                        this->state);
}

std::chrono::milliseconds MissionUploadTick::getWait() const {
    return MISSION_UPLOAD_TICK_WAIT;
}

Tick* MissionUploadTick::tick() {
    auto status = this->mission_uploaded.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        if (!this->mission_uploaded.get()) {
            return new MissionPrepTick(this->state);
        } else {
            return new MissionStartTick(this->state);
        }
    }

    return nullptr;
}
