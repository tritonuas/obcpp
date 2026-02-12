#include "ticks/fly_waypoints.hpp"
#include "ticks/fly_search.hpp"
#include <memory>

#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "utilities/constants.hpp"
#include "utilities/common.hpp"
#include "pathing/environment.hpp"

using namespace std::chrono_literals; // NOLINT

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    :Tick(state, TickID::FlyWaypoints), next_tick(next_tick) {}

void FlyWaypointsTick::init() {
    state->getMav()->startMission();
    state->getCamera()->startStreaming();
    this->last_photo_time = getUnixTime_ms();
}

std::chrono::milliseconds FlyWaypointsTick::getWait() const {
    return FLY_WAYPOINTS_TICK_WAIT;
}

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

    if (isMissionFinished) {
        if (state->config.pathing.laps > 1) {
            state->config.pathing.laps--;
            state->getMav()->startMission();
            return nullptr;
        }

        return next_tick;
    }

    return nullptr;
}
