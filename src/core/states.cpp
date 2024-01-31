#include <memory>
#include <mutex>

#include <loguru.hpp>

#include "core/config.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"
#include "utilities/locks.hpp"

// in future might add to this
MissionState::MissionState() = default;

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

const std::optional<CartesianConverterProto>& MissionState::getCartesianConverter() {
    Lock lock(this->converter_mut);
    
    return this->converter;
}

void MissionState::setCartesianConverter(CartesianConverterProto new_converter) {
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

    std::string old_tick_name = "Null";
    if (tick != nullptr) {
        old_tick_name = tick->getName();
    }
    std::string new_tick_name = "Null";
    if (newTick != nullptr) {
        new_tick_name = newTick->getName();
    }
    LOG_F(INFO, "%s -> %s", old_tick_name.c_str(), new_tick_name.c_str());

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
