#include "ticks/path_gen.hpp"

#include <cassert>
#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "pathing/environment.hpp"
#include "pathing/static.hpp"
#include "protos/obc.pb.h"
#include "ticks/ids.hpp"
#include "ticks/path_validate.hpp"
#include "utilities/logging.hpp"

using namespace std::chrono_literals;  // NOLINT

/**
 * Trims off the first few meters of a path
 *
 * @param[in]       conv     ==> cartesian converter
 * @param[in]       buffer_m ==> number of meters to chop off
 * @param[in, out]  path     ==> path that is changed in place
 */
static void trimPathStart(const std::optional<CartesianConverter<GPSProtoVec>>& conv,
                            double buffer_m, std::vector<GPSCoord>& path) {
    if (buffer_m <= 0.0 || path.size() < 2) {
        LOG_F(WARNING, "Trying to trim path that is too small | trying to trim nothing off a path");
        return;
    }

    double traveled = 0.0;
    for (size_t i = 1; i < path.size(); i++) {
        XYZCoord a = conv->toXYZ(path[i - 1]);
        XYZCoord b = conv->toXYZ(path[i]);
        double seg = a.distanceTo(b);

        if (traveled + seg >= buffer_m) {
            XYZCoord start = a + ((buffer_m - traveled) / seg) * (b - a);
            path.erase(path.begin(), path.begin() + i);
            path.insert(path.begin(), conv->toLatLng(start));
            return;
        }

        traveled += seg;
    }

    // keeps last point (waypoint) if whole path is too short
    LOG_F(WARNING, "Trimming to minimum size");
    path.erase(path.begin(), path.end() - 1);
}


PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

void PathGenTick::init() {
    // TODO: parse, don't validate LOL
    assert(this->state->getCartesianConverter().has_value());

    // environment
    assert(Environment::_valid_region.size() >= 3);
    assert(Environment::_airdrop_zone.size() >= 3);
    // assert(Environment::_mapping_region.size() >= 3);
    assert(Environment::_bounds.first.first < Environment::_bounds.first.second);
    assert(Environment::_bounds.second.first < Environment::_bounds.second.second);

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
        MissionPath init = MissionPath(MissionPath::Type::FORWARD, init_gps);
        double angle1 = calculateFinalAngle(init, this->state->getCartesianConverter());

        std::vector<GPSCoord> next_gps = generateNextWaypointPath(this->state, angle1);
        trimPathStart(this->state->getCartesianConverter(),
                    this->state->config.pathing.upload_distance_buffer_m,
                    next_gps);
        MissionPath next = MissionPath(MissionPath::Type::FORWARD, next_gps);
        double angle2 = calculateFinalAngle(next, this->state->getCartesianConverter());


        std::vector<GPSCoord> coverage_gps = generateSearchPath(this->state, angle2);

        MissionPath coverage;
        if (this->state->config.pathing.coverage.method == AirdropCoverageMethod::Enum::FORWARD) {
            trimPathStart(this->state->getCartesianConverter(),
                        this->state->config.pathing.upload_distance_buffer_m,
                        coverage_gps);
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
