#ifndef INCLUDE_TICKS_WAIT_FOR_TAKEOFF_HPP_
#define INCLUDE_TICKS_WAIT_FOR_TAKEOFF_HPP_

#include <memory>
#include <string>
#include <chrono>

#include "ticks/tick.hpp"

class WaitForTakeoffTick: public Tick {
 public:
    enum class Status {
        None = 0,
        Manual = 1,
        Autonomous = 2,
    };

    explicit WaitForTakeoffTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    void setStatus(Status status);

    Tick* tick() override;

 private:
    Status status;
};

#endif  // INCLUDE_TICKS_WAIT_FOR_TAKEOFF_HPP_
