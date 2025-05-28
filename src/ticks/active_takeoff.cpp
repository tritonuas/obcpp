#include "ticks/active_takeoff.hpp"

#include <memory>
#include <future>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_waypoints.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/fly_search.hpp"

ActiveTakeoffTick::ActiveTakeoffTick(std::shared_ptr<MissionState> state):
    Tick(state, TickID::ActiveTakeoff) {}

std::chrono::milliseconds ActiveTakeoffTick::getWait() const {
    return ACTIVE_TAKEOFF_TICK_WAIT;
}

void ActiveTakeoffTick::init() {
    this->armAndHover();
}

void ActiveTakeoffTick::armAndHover() {
    this->takeoffResult = std::async(std::launch::async,
        [this](){ return this->state->getMav()->armAndHover(this->state); });
}

Tick* ActiveTakeoffTick::tick() {
    auto takeoff = this->takeoffResult.wait_for(std::chrono::milliseconds(0));

    if (takeoff != std::future_status::ready) {
        return nullptr;
    }

    auto result = takeoffResult.get();

    if (result != true) {
        return new MissionPrepTick(this->state);
    }

    LOG_F(INFO, "Vehicle at required altitude");

    auto startMission = this->state->getMav()->startMission();

    if (startMission != true) {
        return new MissionPrepTick(this->state);
    }

    // NOTE: keep in sync with manual takeoff tick
    // transitions to flying waypoints tick, such that when the flying waypoints
    // tick is done it transitions to uploading the coverage path

    if (state->getDroppedAirdrops().size() >= this->state->config.takeoff.payload_size) {
        return new FlyWaypointsTick(this->state, new AirdropPrepTick(this->state));
        // return new AirdropPrepTick(this->state);
    } else {
        return new FlyWaypointsTick(this->state, new MavUploadTick(
            this->state, new FlySearchTick(this->state),
            state->getCoveragePath(), false));
    }
}
