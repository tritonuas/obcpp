#ifndef INCLUDE_TICKS_FLY_WAYPOINTS_HPP_
#define INCLUDE_TICKS_FLY_WAYPOINTS_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"

/*
 * Handles logic during flight of initial waypoints 
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/flywaypoints/
 */
class FlyWaypointsTick : public Tick {
 public:
    explicit FlyWaypointsTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_FLY_WAYPOINTS_HPP_
