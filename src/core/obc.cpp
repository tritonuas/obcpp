#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>

#include <loguru.hpp>

#include "core/obc.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/mission_prep.hpp"
#include "network/gcs.hpp"

// TODO: allow specifying config filename
OBC::OBC(uint16_t gcs_port) {
    this->state = std::make_shared<MissionState>();
    this->state->setTick(new MissionPrepTick(this->state));

    this->gcs_server = std::make_unique<GCSServer>(gcs_port, this->state);
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}
