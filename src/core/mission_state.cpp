#include <memory>
#include <mutex>

#include "core/mission_config.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/message.hpp"
#include "utilities/locks.hpp"
#include "network/mavlink.hpp"
#include "network/airdrop_client.hpp"
#include "utilities/logging.hpp"

// in future might add to this
MissionState::MissionState() = default;

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
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

bool MissionState::sendTickMsg(TickMessage msg) {
    Lock lock(this->tick_msgs_mut);

    if (msg.id != this->tick->getID()) {
        return false;
    }

    this->tick_msgs.push(msg);
    return true;
}

std::optional<TickMessage> MissionState::recvTickMsg() {
    Lock lock(this->tick_msgs_mut);
    if (this->tick_msgs.empty()) {
        return {};
    }

    auto msg = this->tick_msgs.front();
    this->tick_msgs.pop();
    return msg;
}

void MissionState::setInitPath(std::vector<GPSCoord> init_path) {
    Lock lock(this->init_path_mut);
    this->init_path = init_path;
}

const std::vector<GPSCoord>& MissionState::getInitPath() {
    Lock lock(this->init_path_mut);
    return this->init_path;
}

std::shared_ptr<MavlinkClient> MissionState::getMav() {
    return this->mav;
}

void MissionState::setMav(std::shared_ptr<MavlinkClient> mav) {
    this->mav = mav;
}

std::shared_ptr<AirdropClient> MissionState::getAirdrop() {
    return this->airdrop;
}

void MissionState::setAirdrop(std::shared_ptr<AirdropClient> airdrop) {
    this->airdrop = airdrop;
}
