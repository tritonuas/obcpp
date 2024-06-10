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

PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {
    startPathGeneration();
}

void PathGenTick::startPathGeneration() {
    this->init_path = std::async(std::launch::async, generateInitialPath, this->state);
    this->search_path = std::async(std::launch::async, generateSearchPath, this->state);
}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

Tick* PathGenTick::tick() {
    auto init_status = this->init_path.wait_for(0ms);
    auto search_status = this->search_path.wait_for(0ms);
    if (init_status == std::future_status::ready &&
        search_status == std::future_status::ready) {
        LOG_F(INFO, "Initial path generated");
        state->setInitPath(this->init_path.get());
        state->setSearchPath(this->search_path.get());
        return new PathValidateTick(this->state);
    }

    return nullptr;
}
