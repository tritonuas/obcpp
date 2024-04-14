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
OBC::OBC(OBCConfig config) {
    const char* mavlink_url= config.network_mavlink_connect.c_str();
    int gcs_port = config.network_gcs_port;

    LOG_F(INFO, mavlink_url);

    this->state = std::make_shared<MissionState>();
    this->state->setTick(new MissionPrepTick(this->state));

    this->gcs_server = std::make_unique<GCSServer>(gcs_port, this->state);

    // Don't need to look at these futures at all because the connect functions
    // will set the global mission state themselves when connected, which everything
    // else can check.
    this->connectMavThread = std::thread([this, mavlink_url]{this->connectMavlink(mavlink_url);});
    this->connectAirdropThread = std::thread([this]{this->connectAirdrop();});
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}

void OBC::connectMavlink(const char* mavlink_url) {
    loguru::set_thread_name("mav connect");

    // TODO: pull mav ip from config file
    std::shared_ptr<MavlinkClient> mav(new MavlinkClient(mavlink_url));
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
