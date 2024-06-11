#include "ticks/path_gen.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "pathing/static.hpp"
#include "protos/obc.pb.h"
#include "ticks/ids.hpp"
#include "ticks/path_validate.hpp"
#include "utilities/logging.hpp"

using namespace std::chrono_literals;

PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

void PathGenTick::init() {
    startPathGeneration();
}

Tick* PathGenTick::tick() {
    auto init_status = this->init_path.wait_for(0ms);
    auto search_status = this->coverage_path.wait_for(0ms);
    if (init_status == std::future_status::ready &&
        search_status == std::future_status::ready) {
        LOG_F(INFO, "Initial and Coverage paths generated");
        state->setInitPath(this->init_path.get());
        state->setCoveragePath(this->coverage_path.get());
        return new PathValidateTick(this->state);
    }

    return nullptr;
}

void PathGenTick::startPathGeneration() {
    this->init_path = std::async(std::launch::async, generateInitialPath, this->state);
    this->coverage_path = std::async(std::launch::async, generateSearchPath, this->state);
}