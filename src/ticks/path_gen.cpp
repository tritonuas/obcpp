#include "ticks/path_gen.hpp"

#include <memory>
#include <vector>
#include <future>
#include <chrono>

#include <loguru.hpp>

#include "protos/obc.pb.h"
#include "ticks/takeoff_prep.hpp"

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
        return new TakeoffPrepTick(this->state);
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
