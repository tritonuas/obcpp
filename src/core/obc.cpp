#include <memory>
#include <chrono>
#include <thread>

#include "core/obc.hpp"
#include "core/states.hpp"

#include "network/gcs.hpp"

// TODO: allow specifying config filename
OBC::OBC() 
{
    this->config = std::make_shared<MissionConfig>();
    this->state =
        std::make_shared<std::unique_ptr<MissionState>>(
            std::make_unique<PreparationState>(this->config)
        );

    this->gcs_server = std::make_unique<GCSServer>(this->config, this->state);
}

void OBC::run() {
    while (true) {
        state->get()->tick();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}