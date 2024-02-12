#ifndef INCLUDE_TICKS_PATH_VALIDATE_HPP_
#define INCLUDE_TICKS_PATH_VALIDATE_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"
#include "core/mission_state.hpp"

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 */
class PathValidateTick : public Tick {
 public:
    explicit PathValidateTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_PATH_VALIDATE_HPP_
