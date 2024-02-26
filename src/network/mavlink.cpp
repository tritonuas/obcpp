#include "network/mavlink.hpp"

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/geofence/geofence.h>

#include <atomic>
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <cmath>

#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "core/mission_state.hpp"

MavlinkClient::MavlinkClient(const char* link) {
    LOG_SCOPE_F(INFO, "Connecting to Mav at %s", link);

    while (true) {
        LOG_F(INFO, "Attempting to add mav connection...");
        const auto conn_result = this->mavsdk.add_any_connection(link);
        if (conn_result == mavsdk::ConnectionResult::Success) {
            LOG_F(INFO, "Mavlink connection successfully established at %s", link);
            break;
        }

        LOG_S(WARNING) << "Mavlink connection failed: " << conn_result << ". Trying again...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Wait for the system to connect via heartbeat
    while (mavsdk.systems().size() == 0) {
        LOG_F(WARNING, "No heartbeat. Trying again...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    LOG_F(INFO, "Mavlink heartbeat received");

    // System got discovered.
    this->system = mavsdk.systems()[0];

    // Create instance of Telemetry and Mission
    this->telemetry = std::make_unique<mavsdk::Telemetry>(system);
    this->mission = std::make_unique<mavsdk::Mission>(system);
    this->geofence = std::make_unique<mavsdk::Geofence>(system);

    // Set position update rate (1 Hz)
    // TODO: set the 1.0 update rate value in the obc config
    while (true) {
        LOG_F(INFO, "Attempting to set telemetry polling rate to %f...", 1.0);
        const auto set_rate_result = this->telemetry->set_rate_position(1.0);
        if (set_rate_result == mavsdk::Telemetry::Result::Success) {
            LOG_F(INFO, "Successfully set mavlink polling rate to %f", 1.0);
            break;
        } else if (set_rate_result == mavsdk::Telemetry::Result::Unsupported) {
            LOG_F(INFO, "Setting mavlink polling rate Unsupported, so skipping");
            break;
        }

        LOG_S(WARNING) << "Setting mavlink polling rate to 1.0 failed: "
            << set_rate_result << ". Trying again...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    LOG_F(INFO, "Setting mavlink telemetry subscriptions");
    // Set subscription functions to keep internal data struct up to date
    this->telemetry->subscribe_position(
        [this](mavsdk::Telemetry::Position position) {
            VLOG_F(DEBUG, "Latitude: %f\tLongitude: %f\tAGL Alt: %f\tMSL Alt: %f)",
                position.latitude_deg, position.longitude_deg,
                position.relative_altitude_m, position.absolute_altitude_m);

            Lock lock(this->data_mut);
            this->data.altitude_agl_m = position.relative_altitude_m;
            this->data.altitude_msl_m = position.absolute_altitude_m;
            this->data.lat_deg = position.latitude_deg;
            this->data.lng_deg = position.longitude_deg;
        });
    this->telemetry->subscribe_flight_mode(
        [this](mavsdk::Telemetry::FlightMode flight_mode){
            std::ostringstream stream;
            stream << flight_mode;
            VLOG_F(DEBUG, "Mav Flight Mode: %s", stream.str().c_str());

            Lock lock(this->data_mut);
            this->data.flight_mode = flight_mode;
        });
    this->telemetry->subscribe_fixedwing_metrics(
        [this](mavsdk::Telemetry::FixedwingMetrics fwmets) {
            VLOG_F(DEBUG, "Airspeed: %f", fwmets.airspeed_m_s);

            Lock lock(this->data_mut);
            this->data.airspeed_m_s = static_cast<double>(fwmets.airspeed_m_s);
        });
    this->telemetry->subscribe_velocity_ned(
        [this](mavsdk::Telemetry::VelocityNed vned) {
            const double groundspeed_m_s =
                std::sqrt(std::pow(vned.east_m_s, 2) + std::pow(vned.north_m_s, 2));

            VLOG_F(DEBUG, "Groundspeed: %f", groundspeed_m_s);

            Lock lock(this->data_mut);
            this->data.groundspeed_m_s = groundspeed_m_s;
        });
}

bool MavlinkClient::uploadMissionUntilSuccess(std::shared_ptr<MissionState> state) const {
    LOG_SCOPE_F(INFO, "Uploading Mav Mission");

    // Make sure everything is set up
    auto mission = state->config.getCachedMission();
    if (!mission.has_value()) {
        LOG_F(ERROR, "Upload failed - no mission");
        return false;  // todo determine return type
    }
    if (state->getInitPath().empty()) {
        LOG_F(ERROR, "Upload failed - no initial path");
        return false;
    }

    // Parse the flight boundary / geofence information
    mavsdk::Geofence::Polygon flight_bound;
    flight_bound.fence_type = mavsdk::Geofence::Polygon::FenceType::Inclusion;
    for (const auto& coord : mission->flightboundary()) {
        flight_bound.points.push_back(mavsdk::Geofence::Point {
            .latitude_deg {coord.latitude()},
            .longitude_deg {coord.longitude()},
        });
    }

    // Parse the waypoint information
    std::vector<mavsdk::Mission::MissionItem> mission_items;
    for (const auto& coord : state->getInitPath()) {
        mavsdk::Mission::MissionItem new_item {};
        mission_items.push_back(mavsdk::Mission::MissionItem {
            .latitude_deg {coord.latitude()},
            .longitude_deg {coord.longitude()},
            .relative_altitude_m {static_cast<float>(coord.altitude())},
            .is_fly_through {true}
        });
    }

    // Upload to the plane
    // For some reason it seems that the SITL does not accept the geofence upload
    // So we will try this upload 5 times. If it doesn't work, assume we are on the
    // SITL and skip, but we'll do a big scary log message in case this is something
    // important to notice
    // see TODO below, so we can treat the cases separately for testing and for real missions
    int geofence_attempts = 5;
    while (true) {
        LOG_F(INFO, "Sending geofence information...");
        auto geofence_result = this->geofence->upload_geofence({flight_bound});
        if (geofence_result == mavsdk::Geofence::Result::Success) {
            LOG_F(INFO, "Geofence successfully uploaded");
            break;
        }

        LOG_S(WARNING) << "Geofence failed to upload: " << geofence_result;
        if (geofence_attempts == 0) {
            // TODO: in the obc config file, set whether or not it is a real mission.
            // That way we can decide if this should be a FATAL error or not
            LOG_S(ERROR) << "Unable to upload geofence. If this is a real mission THIS IS BAD.";
            break;
        } else {
            geofence_attempts--;
            LOG_F(WARNING, "Trying again...");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // For some reason the sitl won't sometimes accept waypoint missions...
    // but there is no reason to go until it works, so block here
    while (true) {
        LOG_F(INFO, "Sending waypoint information...");

        mavsdk::Mission::MissionPlan plan {
            .mission_items = mission_items,
        };

        // Upload the mission, logging out progress as it gets uploaded
        std::mutex mut;
        mavsdk::Mission::Result upload_status {mavsdk::Mission::Result::Next};
        this->mission->upload_mission_with_progress_async(plan,
            [&upload_status, &mut]
            (mavsdk::Mission::Result result, mavsdk::Mission::ProgressData data) {
                Lock lock(mut);
                upload_status = result;
                if (result == mavsdk::Mission::Result::Next) {
                    LOG_F(INFO, "Upload progress: %f", data.progress);
                }
            });

        // Wait until the mission is fully uploaded
        while (true) {
            {
                Lock lock(mut);
                // Check if it is uploaded
                if (upload_status == mavsdk::Mission::Result::Success) {
                    LOG_F(INFO, "Successfully uploaded the mission. YIPIEE!");
                    return true;
                } else if (upload_status != mavsdk::Mission::Result::Next) {
                    // Neither success nor "next", signalling that there has been an error
                    LOG_S(WARNING) << "Mission upload failed: "
                        << upload_status << ". Trying again...";
                    break;
                }
            }
            // otherwise in progress, so continue this loop
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::pair<double, double> MavlinkClient::latlng_deg() {
    Lock lock(this->data_mut);
    return {this->data.lat_deg, this->data.lng_deg};
}

double MavlinkClient::altitude_agl_m() {
    Lock lock(this->data_mut);
    return this->data.altitude_agl_m;
}

double MavlinkClient::altitude_msl_m() {
    Lock lock(this->data_mut);
    return this->data.altitude_msl_m;
}

double MavlinkClient::groundspeed_m_s() {
    Lock lock(this->data_mut);
    return this->data.groundspeed_m_s;
}

double MavlinkClient::airspeed_m_s() {
    Lock lock(this->data_mut);
    return this->data.airspeed_m_s;
}

double MavlinkClient::heading_deg() {
    Lock lock(this->data_mut);
    return this->data.heading_deg;
}

mavsdk::Telemetry::FlightMode MavlinkClient::flight_mode() {
    Lock lock(this->data_mut);
    return this->data.flight_mode;
}

// to manually send a command which is not implemented by mavsdk
// You need to use `mavlink_passthrough` which has has a few send_command functions
mavsdk::MavlinkPassthrough::Result sendCustomMavlinkCommand(uint8_t target_sysid, uint8_t target_compid, uint16_t command,
                             float param1, float param2, float param3, float param4, float param5,
                             float param6, float param7) {
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_mavlink_passthrough_1_1_command_long.html

    // These two can probably be gotten from the mavlink object, rather than requiring parameter
    // uint8_t target_sysid = 0;
    // uint8_t target_compid = 0;

    // /usr/include/mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h:124
    // auto mavlink_passthrough = mavsdk::MavlinkPassthrough(system); // TODO: do you have to initialize this boi?
    mavsdk::MavlinkPassthrough::CommandLong command =
        mavsdk::MavlinkPassthrough::CommandLong(target_sysid, target_compid, command, param1,
                                                param2, param3, param4, param5, param6, param7);

    mavsdk::Result result = this->mavsdk::MavlinkPassthrough::send_command_long(command);
    return result;
}
