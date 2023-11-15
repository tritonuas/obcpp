#include <memory>
#include <thread>
#include <chrono>

#include "core/obc.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"

#include "network/gcs.hpp"

// TODO: allow specifying config filename
OBC::OBC() 
{
    this->state = std::make_shared<MissionState>();
    this->state->setTick(new TestTick1(this->state));

    this->gcs_server = std::make_unique<GCSServer>(this->state);
}

void OBC::run() {
    while (true) {
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    }
}