#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <future>

#include <loguru.hpp>

#include "core/obc.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/mission_prep.hpp"
#include "network/gcs.hpp"
#include "network/mavlink.hpp"

// TODO: allow specifying config filename
OBC::OBC(uint16_t gcs_port) {
    this->state = std::make_shared<MissionState>();
    this->state->setTick(new MissionPrepTick(this->state));

    this->gcs_server = std::make_unique<GCSServer>(gcs_port, this->state);

    std::async(std::launch::async, &OBC::connectMavlink, this);
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}

void OBC::connectMavlink() {
    // TODO: pull mav ip from config file
    std::shared_ptr<MavlinkClient> mav(new MavlinkClient("/dev/ttyUSB0"));
    this->state->setMav(mav);
}
