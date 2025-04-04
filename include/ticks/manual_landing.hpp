#ifndef INCLUDE_TICKS_MANUAL_LANDING_HPP_
#define INCLUDE_TICKS_MANUAL_LANDING_HPP_

#include <chrono>
#include <memory>

#include "ticks/tick.hpp"

/*
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/manuallanding/
 */
class ManualLandingTick : public Tick {
 public:
    explicit ManualLandingTick(std::shared_ptr<MissionState> state, Tick* next_tick);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

 private:
    Tick* next_tick;
};

#endif  // INCLUDE_TICKS_MANUAL_LANDING_HPP_
