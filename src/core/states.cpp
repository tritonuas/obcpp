#include <memory>
#include <mutex>

#include "core/config.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"
#include "utilities/locks.hpp"

MissionState* PreparationState::tick() {
    // TODO: logic to check for takeoff signal, and double check to
    // make sure that all the mission parameters are valid

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

    return nullptr;
}
