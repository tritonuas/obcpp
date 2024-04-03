#ifndef INCLUDE_TICKS_MANUAL_LANDING_HPP_
#define INCLUDE_TICKS_MANUAL_LANDING_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"

/*
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/manuallanding/
 */
class ManualLandingTick : public Tick {
 public:
    explicit ManualLandingTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_MANUAL_LANDING_HPP_
