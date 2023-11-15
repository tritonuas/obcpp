#ifndef CORE_TICKS_HPP
#define CORE_TICKS_HPP

#include <memory>
#include <chrono>

#include "core/states.hpp"

// When writing tick functions... Absolutely do not do not do not 
// delete the pointer that is being passed in. 

class Tick {
    public:
        Tick(std::shared_ptr<MissionState> state);
        virtual ~Tick();

        // how long to wait between running each tick function
        virtual std::chrono::milliseconds getWait() {
            return std::chrono::milliseconds(1000); // current test default value, codify later...
        }; 

        // function that is called every getWaitTimeMS() miliseconds
        // return nullptr if no state change should happen
        // return new implementation of Tick if state change should happen
        virtual Tick* tick() = 0;

    protected:
        std::shared_ptr<MissionState> state;
};

class TestTick1 : public Tick {
    public:
        TestTick1(std::shared_ptr<MissionState> state);

        Tick* tick() override;
};

class TestTick2 : public Tick {
    public:
        TestTick2(std::shared_ptr<MissionState> state);

        Tick* tick() override;
};

#endif // CORE_TICKS_HPP