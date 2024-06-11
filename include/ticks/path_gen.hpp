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
#include "pathing/mission_path.hpp"

/*
 * Generates a path, caches the path in the mission state,
 * then waits for it to be validated.
 *
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/pathgen/
 */
class PathGenTick : public Tick {
 public:
    explicit PathGenTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
 private:
    std::future<MissionPath> init_path;
    std::future<MissionPath> coverage_path;

    void startPathGeneration();
};

#endif  // INCLUDE_TICKS_PATH_GEN_HPP_
