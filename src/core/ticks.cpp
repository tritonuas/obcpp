#include <memory>
#include <iostream>
#include <future>

#include <loguru.hpp>

#include "core/ticks.hpp"
#include "core/states.hpp"
#include "utilities/constants.hpp"

Tick::Tick(std::shared_ptr<MissionState> state) {
    this->state = state;
}

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
Tick::~Tick() = default;

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

std::vector<GPSCoord> tempGenPath(std::shared_ptr<MissionState> state) {
    // TODO: replace this with the actual path generation function
    // For now , just returns a path with 1 coord, which is technically
    // "valid" because it has more than 0 coords
    LOG_F(INFO, "Dummy path generation step 1...");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG_F(INFO, "Dummy path generation step 2...");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG_F(INFO, "Dummy path generation step 3...");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto cartesian = state->getCartesianConverter().value();
    auto waypoints = state->config.getWaypoints();

    // Just convert back the waypoints to output path for this test
    std::vector<GPSCoord> output_coords;
    output_coords.reserve(waypoints.size());
    for (auto wpt : waypoints) {
        output_coords.push_back(cartesian.toLatLng(wpt));
    }

    return output_coords;
}

PathGenerationTick::PathGenerationTick(std::shared_ptr<MissionState> state)
    :Tick{state}
{
    startPathGeneration();
}

void PathGenerationTick::startPathGeneration() {
    this->path = std::async(std::launch::async, tempGenPath, this->state);
    this->path_generated = false;
}

std::chrono::milliseconds PathGenerationTick::getWait() const {
    return PATH_GEN_TICK_WAIT;
}

Tick* PathGenerationTick::tick() {
    if (this->state->isInitPathValidated()) {
        // Path already validated, so move onto next state
        return nullptr;  // TODO: move onto next state
    }

    if (this->path_generated) {
        // Already generated the path, so no point to be here
        return nullptr;
    }

    auto status = this->path.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        LOG_F(INFO, "Initial path generated");
        state->setInitPath(path.get());
        this->path_generated = true;
    }

    return nullptr;
}

std::string PathGenerationTick::getName() const {
    return "Path Generation";
}