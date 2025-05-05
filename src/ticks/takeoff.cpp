#include "ticks/takeoff.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/fly_search.hpp"
#include "ticks/manual_landing.hpp"
#include "ticks/refueling.hpp"

TakeoffTick::TakeoffTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::Takeoff) {}

std::chrono::milliseconds TakeoffTick::getWait() const {
    return TAKEOFF_TICK_WAIT;
}

Tick* TakeoffTick::tick() {
    if (state->getMav()->flight_mode() == mavsdk::Telemetry::FlightMode::Mission) {
        LOG_F(INFO, "Plane has entered autonomous");

        // NOTE: keep in sync with active_takeoff tick
        // transitions to flying waypoints tick, such that when the flying waypoints
        // tick is done it transitions to uploading the coverage pathi

        // TODO: This is changed for debugging!!!!

        return new FlyWaypointsTick(this->state, new ManualLandingTick(state, new RefuelingTick(state)));
        // return new FlyWaypointsTick(this->state, new MavUploadTick(
        //     this->state, new FlySearchTick(this->state),
        //     state->getCoveragePath(), false));
    }

    return nullptr;
}
