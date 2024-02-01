#ifndef INCLUDE_TICKS_TAKEOFF_PREP_HPP_
#define INCLUDE_TICKS_TAKEOFF_PREP_HPP_

#include <memory>
#include <chrono>
#include <string>

#include "ticks/tick.hpp"

/*
 * Checks every second whether or not a valid mission has been uploaded.
 * Transitions to PathGenerationTick once it has been generated.
 */
class TakeoffPrepTick: public Tick {
 public:
    explicit TakeoffPrepTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

    std::string getName() const override;
};

#endif  // INCLUDE_TICKS_TAKEOFF_PREP_HPP_
