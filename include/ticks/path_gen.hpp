#ifndef INCLUDE_TICKS_PATH_GEN_HPP_
#define INCLUDE_TICKS_PATH_GEN_HPP_

#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <future>

#include "ticks/tick.hpp"
#include "core/mission_state.hpp"
#include "protos/obc.pb.h"

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 */
class PathGenTick : public Tick {
 public:
    explicit PathGenTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
 private:
    std::future<std::vector<GPSCoord>> path;
    bool path_generated;

    void startPathGeneration();
};

#endif  // INCLUDE_TICKS_PATH_GEN_HPP_