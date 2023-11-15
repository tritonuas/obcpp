#include <memory>
#include <iostream>

#include "core/ticks.hpp"
#include "core/states.hpp"

Tick::Tick(std::shared_ptr<MissionState> state) {
    this->state = state;
}

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
Tick::~Tick() = default;

TestTick1::TestTick1(std::shared_ptr<MissionState> state)
    :Tick(state) {}

Tick* TestTick1::tick() {
    std::cout << "tick 1" << std::endl;
    return new TestTick2(this->state);
}

TestTick2::TestTick2(std::shared_ptr<MissionState> state)
    :Tick(state) {}

Tick* TestTick2::tick() {
    std::cout << "tick 2" << std::endl;
    return new TestTick1(this->state);
}