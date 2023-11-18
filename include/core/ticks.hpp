#ifndef CORE_TICKS_HPP
#define CORE_TICKS_HPP

#include <memory>
#include <chrono>
#include <future>

#include "core/states.hpp"

// When writing tick functions... Absolutely do not do not do not 
// delete the pointer that is being passed in. 

class Tick {
    public:
        Tick(std::shared_ptr<MissionState> state);
        virtual ~Tick();

        // how long to wait between running each tick function
        virtual std::chrono::milliseconds getWait() const = 0;

        // function that is called every getWaitTimeMS() miliseconds
        // return nullptr if no state change should happen
        // return new implementation of Tick if state change should happen
        virtual Tick* tick() = 0;

    protected:
        std::shared_ptr<MissionState> state;
};

/*
 * Checks every second whether or not a valid mission has been uploaded.
 * Transitions to PathGenerationTick once it has been generated.
 */
class MissionPreparationTick : public Tick {
    public:
        MissionPreparationTick(std::shared_ptr<MissionState> state);

        std::chrono::milliseconds getWait() const override;

        Tick* tick() override;
};

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 */
class PathGenerationTick : public Tick {
    public:
        PathGenerationTick(std::shared_ptr<MissionState> state);

        std::chrono::milliseconds getWait() const override;

        Tick* tick() override;
    private:
        std::future<std::vector<GPSCoord>> path_future;
};

#endif // CORE_TICKS_HPP