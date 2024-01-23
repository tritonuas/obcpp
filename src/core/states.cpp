#include <memory>
#include <mutex>

#include "core/config.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"
#include "utilities/locks.hpp"

// in future might add to this
MissionState::MissionState() = default;

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

const std::optional<CartesianConverter>& MissionState::getCartesianConverter() {
    Lock lock(this->converter_mut);
    
    return this->converter;
}

void MissionState::setCartesianConverter(CartesianConverter new_converter) {
    Lock lock(this->converter_mut);

    this->converter = new_converter;
}

std::chrono::milliseconds MissionState::doTick() {
    Lock lock(this->tick_mut);

    Tick* newTick = tick->tick();
    if (newTick != nullptr) {
        tick.reset(newTick);
    }

    return tick->getWait();
}

void MissionState::setTick(Tick* newTick) {
    Lock lock(this->tick_mut);

    tick.reset(newTick);
}

void MissionState::setInitPath(std::vector<GPSCoord> init_path) {
    Lock lock(this->init_path_mut);
    this->init_path = init_path;
}

const std::vector<GPSCoord>& MissionState::getInitPath() {
    Lock lock(this->init_path_mut);
    return this->init_path;
}

bool MissionState::isInitPathValidated() {
    Lock lock(this->init_path_mut);
    return this->init_path_validated;
}
