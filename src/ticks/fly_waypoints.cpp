#include "ticks/fly_waypoints.hpp"

#include <chrono>
#include <future>
#include <memory>

#include "pathing/static.hpp"
#include "ticks/fly_search.hpp"
#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"
#include "utilities/common.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    : Tick(state, TickID::FlyWaypoints), next_tick(next_tick) {}

void FlyWaypointsTick::init() {
    // note: I didn't get around to testing if 1 would be a better value than 0
    // to see if the mission start can be forced.
    if (!this->state->getMav()->setMissionItem(1)) {
        LOG_F(ERROR, "Failed to reset Mission");
    }

    this->mission_started = this->state->getMav()->startMission();
    state->getCamera()->startStreaming();
    this->last_photo_time = getUnixTime_ms();

    // I have another one here because idk how startmIssion behaves exactly
    if (!this->state->getMav()->setMissionItem(1)) {
        LOG_F(ERROR, "Failed to reset Mission");
    }

    LOG_F(INFO, "Started FlyWaypointsTick, Laps Remaining: %d", state->getLapsRemaining());
}

std::chrono::milliseconds FlyWaypointsTick::getWait() const { return FLY_WAYPOINTS_TICK_WAIT; }

Tick* FlyWaypointsTick::tick() {
    auto [lat_deg, lng_deg] = state->getMav()->latlng_deg();
    double altitude_agl_m = state->getMav()->altitude_agl_m();
    GPSCoord current_pos = makeGPSCoord(lat_deg, lng_deg, altitude_agl_m);
    // Get the CartesianConverter (which is already initialized from mission boundaries)
    auto converter = state->getCartesianConverter();
    if (converter) {
        // Convert GPS to local XYZ coords
        XYZCoord current_xyz = converter->toXYZ(current_pos);
        // Get the airdrop boundary polygon
        Polygon airdrop_boundary = state->mission_params.getAirdropBoundary();
        // Check if we're inside the airdrop zone
        bool in_zone = Environment::isPointInPolygon(airdrop_boundary, current_xyz);

        if (in_zone) {
            auto now = getUnixTime_ms();
            if ((now - this->last_photo_time) >= 300ms) {
                auto photo = this->state->getCamera()->takePicture(100ms, this->state->getMav());
                if (state->config.camera.save_images_to_file) {
                    photo->saveToFile(state->config.camera.save_dir);
                }

                if (photo.has_value()) {
                    // Update the last photo time
                    this->last_photo_time = getUnixTime_ms();
                    // Run the pipeline on the photo
                    this->state->getCV()->runPipeline(photo.value());
                }
            }
        } else {
            // Stop camera/CV operations
        }
    }

    // TODO: Eventually implement dynamic avoidance so we dont crash brrr
    bool isMissionFinished = state->getMav()->isMissionFinished();

    if (!isMissionFinished) {
        return nullptr;
    }


    if (state->getLapsRemaining() > 1) {
        // regenerate path
        std::future<MissionPath> init_path;
        init_path = std::async(std::launch::async, generateInitialPath, this->state);
        auto init_status = init_path.wait_for(std::chrono::milliseconds(0));
        int count_ms = 2500;
        const int wait_time_ms = 100;

        while (init_status != std::future_status::ready && count_ms > 0) {
            LOG_F(WARNING, "Waiting for path to be generated...");
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
            init_status = init_path.wait_for(std::chrono::milliseconds(0));
            count_ms -= wait_time_ms;
        }

        if (count_ms <= 0) {
            LOG_F(ERROR, "Path generation took too long. Trying Again...");
            return nullptr;
        }

        state->decrementLapsRemaining();
        state->setInitPath(init_path.get());

        return new MavUploadTick(
            this->state, new FlyWaypointsTick(this->state, new FlySearchTick(this->state)),
            state->getInitPath(), false);
    }

    state->decrementLapsRemaining(); // decrement to zero
    return new MavUploadTick(
        this->state, new FlySearchTick(this->state),
        state->getCoveragePath(), false);
}
