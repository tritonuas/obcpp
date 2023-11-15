#include <iostream>
#include <memory>

#include "core/config.hpp"
#include "core/states.hpp"

MissionState::MissionState(std::shared_ptr<MissionConfig> config) {
    this->config = config;
}

PreparationState::PreparationState(std::shared_ptr<MissionConfig> config) 
    :MissionState{config} {}

MissionState* PreparationState::tick() {
    // TODO: logic to check for takeoff signal, and double check to make sure that
    // all the mission parameters are valid

    std::cout << "tick\n";

    return nullptr;
}