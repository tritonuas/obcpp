#ifndef INCLUDE_TICKS_AIRDROP_APPROACH_HPP_
#define INCLUDE_TICKS_AIRDROP_APPROACH_HPP_

#include <chrono>
#include <memory>

#include "ticks/tick.hpp"

/*
 * Fly to aidrop points and communicate with airdrop mechanism.
 *
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/airdropapproach/
 */
class AirdropApproachTick : public Tick {
 public:
    explicit AirdropApproachTick(std::shared_ptr<MissionState> state);

    void init() override;

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

void triggerAirdrop(std::shared_ptr<MavlinkClient> mav, airdrop_t airdrop_index);

#endif  // INCLUDE_TICKS_AIRDROP_APPROACH_HPP_
