#include "core/mission_state.hpp"

#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

#include "camera/interface.hpp"
#include "camera/mock.hpp"
#include "core/mission_parameters.hpp"
#include "cv/aggregator.hpp"
#include "cv/pipeline.hpp"
#include "network/airdrop_client.hpp"
#include "network/mavlink.hpp"
#include "ticks/tick.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"

MissionState::MissionState(OBCConfig config): config(config) {}

// Need to explicitly define now that Tick is no longer an incomplete class
// See:
// https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

const std::optional<CartesianConverter<GPSProtoVec>>& MissionState::getCartesianConverter() {
    Lock lock(this->converter_mut);

    return this->converter;
}

void MissionState::setCartesianConverter(CartesianConverter<GPSProtoVec> new_converter) {
    Lock lock(this->converter_mut);

    this->converter = new_converter;
}

std::chrono::milliseconds MissionState::doTick() {
    Lock lock(this->tick_mut);

    Tick* newTick = this->tick->tick();
    if (newTick) {
        this->_setTick(newTick);
    }

    return this->tick->getWait();
}

void MissionState::setTick(Tick* newTick) {
    Lock lock(this->tick_mut);

    this->_setTick(newTick);
}

void MissionState::_setTick(Tick* newTick) {
    std::string old_tick_name = (this->tick) ? this->tick->getName() : "Null";
    std::string new_tick_name = (newTick) ? newTick->getName() : "Null";

    LOG_F(INFO, "%s -> %s", old_tick_name.c_str(), new_tick_name.c_str());

    tick.reset(newTick);
}

TickID MissionState::getTickID() {
    Lock lock(this->tick_mut);
    return this->tick->getID();
}

void MissionState::setInitPath(std::vector<GPSCoord> init_path) {
    Lock lock(this->init_path_mut);
    this->init_path = init_path;
}

const std::vector<GPSCoord>& MissionState::getInitPath() {
    Lock lock(this->init_path_mut);
    return this->init_path;
}

void MissionState::setSearchPath(std::vector<GPSCoord> search_path) {
    Lock lock(this->search_path_mut);
    this->search_path = search_path;
}

const std::vector<GPSCoord>& MissionState::getSearchPath() {
    Lock lock(this->search_path_mut);
    return this->search_path;
}

std::shared_ptr<MavlinkClient> MissionState::getMav() { return this->mav; }

void MissionState::setMav(std::shared_ptr<MavlinkClient> mav) { this->mav = mav; }

std::shared_ptr<AirdropClient> MissionState::getAirdrop() { return this->airdrop; }

void MissionState::setAirdrop(std::shared_ptr<AirdropClient> airdrop) { this->airdrop = airdrop; }

std::shared_ptr<CVAggregator> MissionState::getCV() { return this->cv; }

void MissionState::setCV(std::shared_ptr<CVAggregator> cv) { this->cv = cv; }

std::shared_ptr<CameraInterface> MissionState::getCamera() { return this->camera; }

void MissionState::setCamera(std::shared_ptr<CameraInterface> camera) { this->camera = camera; }
