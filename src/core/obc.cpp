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

    // Don't need to look at these futures at all because the connect functions
    // will set the global mission state themselves when connected, which everything
    // else can check.
    auto _fut1 = std::async(std::launch::async, &OBC::connectMavlink, this);
    auto _fut2 = std::async(std::launch::async, &OBC::connectAirdrop, this);
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}

void OBC::connectMavlink() {
    // TODO: pull mav ip from config file
    std::shared_ptr<MavlinkClient> mav(new MavlinkClient("serial:///dev/ttyACM0"));
    // std::shared_ptr<MavlinkClient> mav(new MavlinkClient("tcp://172.17.0.1:5760"));
    this->state->setMav(mav);
}

void OBC::connectAirdrop() {
    ad_socket_result_t result;
    while (true) {
        result = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT);
        if (!result.is_err) {
            break;
        }

        LOG_F(ERROR, "Failed to establish airdrop socket: %s. Trying again in 3 seconds...",
            result.data.err);
    }

    this->state->setAirdrop(std::make_shared<AirdropClient>(result.data.res));
}
