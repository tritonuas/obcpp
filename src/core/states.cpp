#include <iostream>

#include "core/states.hpp"

MissionState* PreparationState::tick() {
    // TODO: logic to check for takeoff signal, and double check to make sure that
    // all the mission parameters are valid

    std::cout << "tick\n";

    return nullptr;
}