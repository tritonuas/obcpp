#ifndef INCLUDE_TICKS_REFUELING_HPP_
#define INCLUDE_TICKS_REFUELING_HPP_

#include <chrono>
#include <memory>

#include "ticks/tick.hpp"

/*
 * Wait for the refueling process to finish.
 */
class RefuelingTick : public Tick {
 public:
    explicit RefuelingTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_REFUELING_HPP_