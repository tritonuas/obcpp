#include "ticks/tick.hpp"

#include <memory>

#include "core/states.hpp"

Tick::Tick(std::shared_ptr<MissionState> state) {
    this->state = state;
}

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
Tick::~Tick() = default;