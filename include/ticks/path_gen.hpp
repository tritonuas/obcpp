#ifndef INCLUDE_CORE_TICKS_PATH_GEN_HPP_
#define INCLUDE_CORE_TICKS_PATH_GEN_HPP_

#include "ticks/tick.hpp"
#include "core/mission_state.hpp"
#include "protos/obc.pb.h"

#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <future>

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 */
class PathGenerationTick : public Tick {
 public:
    explicit PathGenerationTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

    std::string getName() const override;
 private:
    std::future<std::vector<GPSCoord>> path;
    bool path_generated;

    void startPathGeneration();
};

#endif