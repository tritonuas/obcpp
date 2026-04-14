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

using namespace std::chrono_literals;  // NOLINT

PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

void PathGenTick::init() {
    startPathGeneration();
}

Tick* PathGenTick::tick() {
    auto status = this->paths_future.wait_for(0ms);
    if (status == std::future_status::ready) {
        LOG_F(INFO, "Initial and Coverage paths generated");
        return new PathValidateTick(this->state);
    }

    return nullptr;
}

void PathGenTick::startPathGeneration() {
    this->paths_future = std::async(std::launch::async, [this]() {
        MissionPath init = generateInitialPath(this->state);
        double angle1 = calculateFinalAngle(init, this->state);
        
        MissionPath next = generateNextWaypointPath(this->state, angle1);
        double angle2 = calculateFinalAngle(next, this->state);
        
        MissionPath coverage = generateSearchPath(this->state, angle2);
        
        this->state->setInitPath(init);
        this->state->setNextWaypointPath(next);
        this->state->setCoveragePath(coverage);
    });
}
