#ifndef INCLUDE_TICKS_ACTIVETAKEOFF_HPP_
#define INCLUDE_TICKS_ACTIVETAKEOFF_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"

/*
 * Waits until the plane has been switched into autopilot mode, then switches to the 
 * next state. IF we ever implement autonomous takeoff, this will need to be replaced.
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/takeoff/
 */
class ActiveTakeoffTick: public Tick {
 public:
    explicit ActiveTakeoffTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_ACTIVETAKEOFF_HPP_