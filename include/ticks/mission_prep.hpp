#ifndef INCLUDE_TICKS_MISSION_PREP_HPP_
#define INCLUDE_TICKS_MISSION_PREP_HPP_

#include <memory>
#include <chrono>
#include <string>

#include "ticks/tick.hpp"

/*
 * Checks every second whether or not a valid mission has been uploaded.
 * Transitions to PathGenTick once it has been generated.
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/missionprep/
 */
class MissionPrepTick : public Tick {
 public:
    explicit MissionPrepTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_MISSION_PREP_HPP_
