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
    assert(this->state->getCartesianConverter().has_value());
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
        const int NUM_WAYPOINTS_TO_REMOVE =
            std::floor(this->state->config.pathing.upload_distance_buffer_m /
                      this->state->config.pathing.dubins.point_separation);

        std::vector<GPSCoord> init_gps = generateInitialPath(this->state);
        MissionPath init = MissionPath(MissionPath::Type::FORWARD, init_gps);
        double angle1 = calculateFinalAngle(init, this->state->getCartesianConverter());

        std::vector<GPSCoord> next_gps = generateNextWaypointPath(this->state, angle1);
        assert(next_gps.size() >= NUM_WAYPOINTS_TO_REMOVE);
        next_gps.erase(next_gps.begin(), next_gps.begin() + NUM_WAYPOINTS_TO_REMOVE);;
        MissionPath next = MissionPath(MissionPath::Type::FORWARD, next_gps);
        double angle2 = calculateFinalAngle(next, this->state->getCartesianConverter());


        std::vector<GPSCoord> coverage_gps = generateSearchPath(this->state, angle2);
        assert(coverage_gps.size() >= NUM_WAYPOINTS_TO_REMOVE);
        coverage_gps.erase(coverage_gps.begin(), coverage_gps.begin() + NUM_WAYPOINTS_TO_REMOVE);;

        MissionPath coverage;
        if (this->state->config.pathing.coverage.method == AirdropCoverageMethod::Enum::FORWARD) {
            coverage = MissionPath(MissionPath::Type::FORWARD, coverage_gps);
        } else {
            coverage = MissionPath(MissionPath::Type::HOVER, coverage_gps,
                                   this->state->config.pathing.coverage.hover.hover_time_s);
        }

        this->state->setInitPath(init);
        this->state->setNextWaypointPath(next);
        this->state->setCoveragePath(coverage);
    });
}
