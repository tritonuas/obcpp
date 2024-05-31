#include "ticks/fly_waypoints.hpp"
#include "ticks/fly_search.hpp"
#include <memory>

#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlyWaypoints) {}

std::chrono::milliseconds FlyWaypointsTick::getWait() const {
    return FLY_WAYPOINTS_TICK_WAIT;
}

Tick* FlyWaypointsTick::tick() {
    // TODO: Eventually implement dynamic avoidance so we dont crash brrr   
    bool isMissionFinished = state->getMav()->isMissionFinished();              \

    if (isMissionFinished) {
        return new MavUploadTick(this->state, new FlySearchTick(this->state),   
                state->getSearchPath(), false);
    }

    return nullptr;
}
