#ifndef INCLUDE_TICKS_MISSION_START_HPP_
#define INCLUDE_TICKS_MISSION_START_HPP_

#include <memory>
#include <chrono>
#include <string>
#include <future>

#include "ticks/tick.hpp"

/*
 * Waits until the plane has been switched into autopilot mode, then switches to the 
 * next state. IF we ever implement autonomous takeoff, this will need to be replaced.
 */
class MissionStartTick: public Tick {
 public:
    explicit MissionStartTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_MISSION_START_HPP_
