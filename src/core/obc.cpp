#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <future>

#include "core/obc.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/mission_prep.hpp"
#include "network/gcs.hpp"
#include "network/mavlink.hpp"
#include "network/airdrop_client.hpp"
#include "utilities/logging.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}

// TODO: allow specifying config filename
OBC::OBC(uint16_t gcs_port) {
    this->state = std::make_shared<MissionState>();
    this->state->setTick(new MissionPrepTick(this->state));

    this->gcs_server = std::make_unique<GCSServer>(gcs_port, this->state);

    this->connectMavThread = std::thread([this]{this->connectMavlink();});
    this->connectAirdropThread = std::thread([this]{this->connectAirdrop();});
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}

void OBC::connectMavlink() {
    loguru::set_thread_name("mav connect");

    // TODO: pull mav ip from config file
    std::shared_ptr<MavlinkClient> mav(new MavlinkClient("serial:///dev/ttyACM0:57600"));
    // std::shared_ptr<MavlinkClient> mav(new MavlinkClient("tcp://172.17.0.1:5760"));
    this->state->setMav(mav);
}

void OBC::connectAirdrop() {
    loguru::set_thread_name("airdrop connect");

    ad_socket_result_t result;
    while (true) {
        LOG_F(INFO, "Attempting to create airdrop socket.");
        result = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT);
        if (!result.is_err) {
            LOG_F(INFO, "Established airdrop socket.");
            break;
        }

        LOG_F(ERROR, "Failed to establish airdrop socket: %s. Trying again in 3 seconds...",
            result.data.err);
    }

    this->state->setAirdrop(std::make_shared<AirdropClient>(result.data.res));
}
