#include "ticks/mav_upload.hpp"

#include <memory>
#include <string>
#include <future>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/takeoff.hpp"
#include "network/mavlink.hpp"
#include "protos/obc.pb.h"

MavUploadTick::MavUploadTick(std::shared_ptr<MissionState> state,
    Tick* next_tick, const MissionPath& waypoints, bool upload_geofence):
        Tick(state, TickID::MavUpload), next_tick{next_tick},
        waypoints{waypoints}, upload_geofence{upload_geofence}
{

}

std::chrono::milliseconds MavUploadTick::getWait() const {
    return MAV_UPLOAD_TICK_WAIT;
}

void MavUploadTick::init() {
    this->mav_uploaded = std::async(std::launch::async,
                                    &MavlinkClient::uploadMissionUntilSuccess,
                                    this->state->getMav(),
                                    this->state,
                                    upload_geofence,
                                    waypoints);
}

Tick* MavUploadTick::tick() {
    auto status = this->mav_uploaded.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        if (!this->mav_uploaded.get()) {
            // try again
            return new MavUploadTick(this->state, this->next_tick,
                this->waypoints, this->upload_geofence);
        } else {
            return this->next_tick;
        }
    }

    return nullptr;
}
