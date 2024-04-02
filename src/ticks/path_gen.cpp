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

PathGenTick::PathGenTick(std::shared_ptr<MissionState> state) : Tick(state, TickID::PathGen) {
    startPathGeneration();
}

void PathGenTick::startPathGeneration() {
    this->path = std::async(std::launch::async, generateInitialPath, this->state);
}

std::chrono::milliseconds PathGenTick::getWait() const { return PATH_GEN_TICK_WAIT; }

Tick* PathGenTick::tick() {
    auto status = this->path.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        LOG_F(INFO, "Initial path generated");
        state->setInitPath(path.get());
        return new PathValidateTick(this->state);
    }

    return nullptr;
}
