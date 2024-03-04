#ifndef INCLUDE_TICKS_PATH_VALIDATE_HPP_
#define INCLUDE_TICKS_PATH_VALIDATE_HPP_

#include <memory>
#include <chrono>
#include <mutex>

#include "ticks/tick.hpp"
#include "core/mission_state.hpp"

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 */
class PathValidateTick : public Tick {
 public:
    enum class Message {
        Rejected = -1, // user rejects the path, return to path generation and try again
        None = 0, // user has not said anything yet, wait
        Validated = 1, // user accepts the path, continue to mission upload
    };

    explicit PathValidateTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

    void setStatus(Message status);
    Message getStatus();

 private:
    Message status;
    std::mutex status_mut;
};

#endif  // INCLUDE_TICKS_PATH_VALIDATE_HPP_
