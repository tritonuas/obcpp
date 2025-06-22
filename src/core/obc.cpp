#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <future>

#include "camera/mock.hpp"
#include "camera/picamera.hpp"
#include "core/obc.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/mission_prep.hpp"
#include "network/gcs.hpp"
#include "network/mavlink.hpp"
#include "network/airdrop_client.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}

// TODO: allow specifying config filename
OBC::OBC(OBCConfig config) {
    int gcs_port = config.network.gcs.port;

    this->state = std::make_shared<MissionState>(config);
    this->state->setTick(new MissionPrepTick(this->state));
    this->state->config = config;
    this->gcs_server = std::make_unique<GCSServer>(gcs_port, this->state);

    // Don't need to look at these futures at all because the connect functions
    // will set the global mission state themselves when connected, which everything
    // else can check.
    this->connectMavThread = std::thread([this, config]
        {this->connectMavlink(config.network.mavlink.connect);});
    this->connectAirdropThread = std::thread([this]{this->connectAirdrop();});

    if (this->state->config.camera.type == "mock") {
        this->state->setCamera(std::make_shared<MockCamera>(this->state->config.camera));
    } else if (this->state->config.camera.type == "picamera") {
        this->state->setCamera(std::make_shared<PiCamera>(this->state->config.camera));
    } else if (this->state->config.camera.type == "picamera-1080p") {
        this->state->setCamera(std::make_shared<PiCamera>(this->state->config.camera, 1920, 1080, 60));
    } else if (this->state->config.camera.type == "picamera-4k") {
        this->state->setCamera(std::make_shared<PiCamera>(this->state->config.camera, 4956, 3040, 30));
    } else if (this->state->config.camera.type == "lucid") {
        LOG_F(FATAL, "ATTEMPTING TO CONNECT TO LUCID CAMERA: LUCID CAMERA NO LONGER EXISTS");
    } else {
        // default to mock if it's neither "mock" or "lucid"
        this->state->setCamera(std::make_shared<MockCamera>(this->state->config.camera));
        LOG_F(WARNING, "deafault camera config");
    }
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}

void OBC::connectMavlink(std::string mavlink_url) {
    loguru::set_thread_name("mav connect");

    // TODO: pull mav ip from config file
    std::shared_ptr<MavlinkClient> mav(new MavlinkClient(this->state->config));
    this->state->setMav(mav);
}

void OBC::connectAirdrop() {
    loguru::set_thread_name("airdrop connect");

    ad_socket_result_t result;
    while (true) {
        LOG_F(INFO, "Attempting to create airdrop socket.");
        result = make_ad_socket(UDP2_OBC_PORT, UDP2_PAYLOAD_PORT);
        if (!result.is_err) {
            LOG_F(INFO, "Established airdrop socket.");
            break;
        }

        LOG_F(ERROR, "Failed to establish airdrop socket: %s. Trying again in 3 seconds...",
            result.data.err);
    }

    this->state->setAirdrop(std::make_shared<AirdropClient>(result.data.res));
}
