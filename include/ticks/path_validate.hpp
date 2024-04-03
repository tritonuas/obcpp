#ifndef INCLUDE_TICKS_PATH_VALIDATE_HPP_
#define INCLUDE_TICKS_PATH_VALIDATE_HPP_

#include <memory>
#include <chrono>
#include <mutex>

#include "ticks/tick.hpp"
#include "core/mission_state.hpp"

/*
 * Check validation of the generated path by the GCS. 
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/pathvalidate/
 */
class PathValidateTick : public Tick {
 public:
    enum class Status {
        Rejected = -1,  // user rejects the path, return to path generation and try again
        None = 0,
        Validated = 1,  // user accepts the path, continue to mission upload
    };

    explicit PathValidateTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    void setStatus(Status status);

    Tick* tick() override;

 private:
    Status status;
};

#endif  // INCLUDE_TICKS_PATH_VALIDATE_HPP_
