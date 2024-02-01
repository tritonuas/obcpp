#ifndef INCLUDE_CORE_TICKS_MISSION_PREP_HPP_
#define INCLUDE_CORE_TICKS_MISSION_PREP_HPP_

#include "ticks/tick.hpp"

#include <memory>
#include <chrono>
#include <string>

/*
 * Checks every second whether or not a valid mission has been uploaded.
 * Transitions to PathGenerationTick once it has been generated.
 */
class MissionPreparationTick : public Tick {
 public:
    explicit MissionPreparationTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

    std::string getName() const override;
};

#endif