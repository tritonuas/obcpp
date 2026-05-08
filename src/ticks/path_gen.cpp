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
        std::vector<GPSCoord> init_gps = generateInitialPath(this->state);
        
        double angle1 = calculateFinalAngle(init_gps, this->state);
        
        std::vector<GPSCoord> next_gps = generateNextWaypointPath(this->state, angle1);
        init_gps.reserve(init_gps.size() + (state->config.pathing.laps-1) * next_gps.size());
        for (int i = 0; i < state->config.pathing.laps-1; i++) {
            init_gps.insert(init_gps.end(), next_gps.begin(), next_gps.end());
        }
        double angle2 = calculateFinalAngle(next_gps, this->state);

        int num_waypoints_to_remove =
            std::ceil(this->state->config.pathing.upload_distance_buffer_m /
                      this->state->config.pathing.dubins.point_separation);
         std::vector<GPSCoord> coverage_gps = generateSearchPath(this->state, angle2);
         coverage_gps.erase(coverage_gps.begin(), coverage_gps.begin() + num_waypoints_to_remove);;

        MissionPath coverage;
        if (this->state->config.pathing.coverage.method == AirdropCoverageMethod::Enum::FORWARD) {
            coverage = MissionPath(MissionPath::Type::FORWARD, coverage_gps);
        } else {
            coverage = MissionPath(MissionPath::Type::HOVER, coverage_gps,
                                   this->state->config.pathing.coverage.hover.hover_time_s);
        }
        MissionPath waypoint = MissionPath(MissionPath::Type::FORWARD, init_gps);
        this->state->setInitPath(waypoint);
        this->state->setNextWaypointPath(waypoint); // TO DO: DELETE NEXT WAYPOINT PATH
        this->state->setCoveragePath(coverage);
    });
}
