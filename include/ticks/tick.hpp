#ifndef INCLUDE_TICKS_TICK_HPP_
#define INCLUDE_TICKS_TICK_HPP_

#include <memory>
#include <chrono>
#include <string>

#include "core/mission_state.hpp"

// When writing tick functions... Absolutely do not do not do not
// delete the pointer that is being passed in.

class Tick {
 public:
    explicit Tick(std::shared_ptr<MissionState> state);
    virtual ~Tick();

    // how long to wait between running each tick function
    virtual std::chrono::milliseconds getWait() const = 0;

    // function that is called every getWaitTimeMS() miliseconds
    // return nullptr if no state change should happen
    // return new implementation of Tick if state change should happen
    virtual Tick* tick() = 0;

    virtual std::string getName() const = 0;
 protected:
    std::shared_ptr<MissionState> state;
};



#endif  // INCLUDE_TICKS_TICK_HPP_
