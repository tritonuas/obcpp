#ifndef INCLUDE_TICKS_AUTO_LANDING_HPP_
#define INCLUDE_TICKS_AUTO_LANDING_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"

/*
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/autolanding/
 */
class AutoLandingTick : public Tick {
 public:
    explicit AutoLandingTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_AUTO_LANDING_HPP_
