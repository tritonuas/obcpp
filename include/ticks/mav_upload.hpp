#ifndef INCLUDE_TICKS_MAV_UPLOAD_HPP_
#define INCLUDE_TICKS_MAV_UPLOAD_HPP_

#include <memory>
#include <vector>
#include <chrono>
#include <string>
#include <future>
#include <optional>

#include "ticks/tick.hpp"
#include "ticks/ids.hpp"
#include "protos/obc.pb.h"
#include "pathing/mission_path.hpp"

/*
 * Handles uploading waypoint mission to the Pixhawk flight
 * controller over Mavlink messages.
 *
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/mavupload/
 */
class MavUploadTick: public Tick {
 public:
    MavUploadTick(std::shared_ptr<MissionState> state,
        Tick* next_tick, const MissionPath& waypoints, bool upload_geofence);

    std::chrono::milliseconds getWait() const override;

    void init() override;
    Tick* tick() override;

 private:
    std::future<bool> mav_uploaded;

    Tick* next_tick;
    MissionPath waypoints;
    bool upload_geofence;
};

#endif  // INCLUDE_TICKS_MAV_UPLOAD_HPP_
