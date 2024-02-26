#include "ticks/path_gen.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "pathing/static.hpp"
#include "protos/obc.pb.h"
#include "ticks/ids.hpp"
#include "ticks/path_validate.hpp"
#include "utilities/logging.hpp"

std::vector<GPSCoord> tempGenPath(std::shared_ptr<MissionState> state) {
    // TODO: replace this with the actual path generation function
    // For now , just returns a path with 1 coord, which is technically
    // "valid" because it has more than 0 coords
    LOG_F(INFO, "Dummy path generation...");

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

PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {
    startPathGeneration();
}

void PathGenTick::startPathGeneration() {
    this->path = std::async(std::launch::async, generateInitialPath, this->state);
}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

Tick* PathGenTick::tick() {
    auto status = this->path.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        LOG_F(INFO, "Initial path generated");
        state->setInitPath(path.get());
        return new PathValidateTick(this->state);
    }

    return nullptr;
}
