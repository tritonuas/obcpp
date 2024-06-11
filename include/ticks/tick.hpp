#ifndef INCLUDE_TICKS_TICK_HPP_
#define INCLUDE_TICKS_TICK_HPP_

#include <memory>
#include <chrono>
#include <string>
#include <mutex>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "utilities/logging.hpp"

class Tick {
 public:
    Tick(std::shared_ptr<MissionState> state, TickID id) {
        this->state = state;
        this->id = id;
        this->already_validated = false;
    }
    virtual ~Tick() = default;

    // how long to wait between running each tick function
    virtual std::chrono::milliseconds getWait() const = 0;

    /**
     * function that is called every getWaitTimeMS() miliseconds
     * return nullptr if no state change should happen
     * return new implementation of Tick if state change should happen
     */
    virtual Tick* tick() = 0;

    /**
     * Code that should be run upon entering the tick should be placed here, not in the constructor
     * because the constructor can possibly be called before actually transitioning into the tick,
     * as is the case for 
     */
    virtual void init() {}

    constexpr TickID getID() const { return this->id; }
    constexpr const char* getName() const { return TICK_ID_TO_STR(this->id); }

 protected:
    std::shared_ptr<MissionState> state;
    TickID id;
    bool already_validated;
};

#endif  // INCLUDE_TICKS_TICK_HPP_
