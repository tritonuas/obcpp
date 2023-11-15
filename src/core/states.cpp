#include <memory>

#include "core/config.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"

// in future might add to this
MissionState::MissionState() = default;

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

const MissionConfig& MissionState::getConfig() {
    return this->config;
}

std::chrono::milliseconds MissionState::doTick() {
    this->tick_mut.lock();

    Tick* newTick = tick->tick();
    if (newTick != nullptr) {
        tick.reset(newTick);
    }

    auto wait = tick->getWait();

    this->tick_mut.unlock();

    return wait;
}

void MissionState::setTick(Tick* newTick) {
    this->tick_mut.lock();

    tick.reset(newTick);

    this->tick_mut.unlock();
}