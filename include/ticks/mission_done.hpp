#ifndef INCLUDE_TICKS_MISSION_DONE_HPP_
#define INCLUDE_TICKS_MISSION_DONE_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"

/*
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/missiondone/
 */
class MissionDoneTick : public Tick {
 public:
    explicit MissionDoneTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_MISSION_DONE_HPP_
