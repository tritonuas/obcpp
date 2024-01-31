#include <memory>
#include <iostream>

#include <loguru.hpp>

#include "core/ticks.hpp"
#include "core/states.hpp"
#include "utilities/constants.hpp"

Tick::Tick(std::shared_ptr<MissionState> state) {
    this->state = state;
}

// Need to explicitly define now that Tick is no longer an incomplete class
// See: https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
Tick::~Tick() = default;

MissionPreparationTick::MissionPreparationTick(std::shared_ptr<MissionState> state)
    :Tick(state) {}

std::chrono::milliseconds MissionPreparationTick::getWait() const {
    return MISSION_PREP_TICK_WAIT;
}

std::string MissionPreparationTick::getName() const {
    return "Mission Preparation";
}

Tick* MissionPreparationTick::tick() {
    if (this->state->config.isValid()) {
        LOG_F(INFO, "Valid mission configuration detected");
        return new PathGenerationTick(this->state);
    } else {
        return nullptr;
    }
}

std::vector<GPSCoord> tempGenPath(std::shared_ptr<MissionState> state) {
    // TODO: replace this with the actual path generation function
    // For now , just returns a path with 1 coord, which is technically
    // "valid" because it has more than 0 coords
    std::this_thread::sleep_for(std::chrono::seconds(10));

    GPSCoord coord;
    coord.set_altitude(0.0);
    coord.set_latitude(1.1);
    coord.set_longitude(2.2);
    return {coord};
}

PathGenerationTick::PathGenerationTick(std::shared_ptr<MissionState> state)
    :Tick{state}
{
    // Essentially, we create an asynchronous function that gets run in another thread.
    // The result of this function is accessible inside of the future member variable
    // we set.
    std::packaged_task<std::vector<GPSCoord>(std::shared_ptr<MissionState>)> path_task(tempGenPath);

    this->path_future = path_task.get_future();

    std::thread thread(std::move(path_task), std::ref(state));
}

std::chrono::milliseconds PathGenerationTick::getWait() const {
    return PATH_GEN_TICK_WAIT;
}

Tick* PathGenerationTick::tick() {
    if (state->isInitPathValidated()) {
        // Path already validated, so move onto next state
        return nullptr;  // TODO: move onto next state
    }

    auto status = path_future.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        state->setInitPath(path_future.get());
        // Set the path, but not validated yet. That must come from an HTTP req
        // from the GCS
    }

    return nullptr;
}

std::string PathGenerationTick::getName() const {
    return "Path Generation";
}