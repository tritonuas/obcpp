#ifndef INCLUDE_TICKS_AIRDROP_PREP_HPP_
#define INCLUDE_TICKS_AIRDROP_PREP_HPP_

#include <memory>
#include <chrono>
#include <string>

#include "ticks/tick.hpp"

/*
 * Determines the approach path of the next airdrop target, or determines that we are finished
 * with the airdrop task.
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/airdropprep/
 */
class AirdropPrepTick: public Tick {
 public:
    explicit AirdropPrepTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_AIRDROP_PREP_HPP_
