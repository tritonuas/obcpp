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

#include <loguru.hpp>

#include "utilities/locks.hpp"

MavlinkClient::MavlinkClient(const char* link) {
    LOG_SCOPE_F(INFO, "Connecting to Mav at %s", link);

    while (true) {
        LOG_F(INFO, "Attempting to add mav connection...");
        const auto conn_result = this->mavsdk.add_any_connection(link);
        if (conn_result == mavsdk::ConnectionResult::Success) {
            LOG_F(INFO, "Mavlink connection successfully established at %s", link);
            break;
        }

        LOG_F(WARNING, "Mavlink connection failed with code %d. Trying again...",
            static_cast<int>(conn_result));
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
    while (true) {
        LOG_F(INFO, "Attempting to set telemetry polling rate to %f...", TELEMETRY_UPDATE_RATE);
        const auto set_rate_result = this->telemetry->set_rate_position(TELEMETRY_UPDATE_RATE);
        if (set_rate_result == mavsdk::Telemetry::Result::Success) {
            LOG_F(INFO, "Successfully set mavlink polling rate to %f", TELEMETRY_UPDATE_RATE);
            break;
        }

        LOG_F(WARNING, "Setting mavlink polling rate to %f failed with code %d. Trying again...",
            TELEMETRY_UPDATE_RATE, static_cast<int>(set_rate_result));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MavlinkClient::uploadMissionUntilSuccess(MissionConfig& config) const {
    LOG_SCOPE_F(INFO, "Uploading Mav Mission");

    auto mission = config.getCachedMission();
    if (!mission.has_value()) {
        return;  // todo determine return type
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
    for (const auto& coord : mission->waypoints()) {
        mavsdk::Mission::MissionItem new_item {};
        mission_items.push_back(mavsdk::Mission::MissionItem {
            .latitude_deg {coord.latitude()},
            .longitude_deg {coord.longitude()},
            .relative_altitude_m {static_cast<float>(coord.altitude())},
            .is_fly_through {true}
        });
    }

    // Upload to the plane
    while (true) {
        LOG_F(INFO, "Sending geofence information...");
        auto geofence_result = this->geofence->upload_geofence({flight_bound});
        if (geofence_result == mavsdk::Geofence::Result::Success) {
            LOG_F(INFO, "Geofence successfully uploaded");
            break;
        }

        LOG_F(WARNING, "Geofence failed to upload with code %d. Trying again...",
            static_cast<int>(geofence_result));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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
                    return;
                } else if (upload_status != mavsdk::Mission::Result::Next) {
                    // Neither success nor "next", signalling that there has been an error
                    LOG_F(WARNING, "Mission upload failed with code %d. Trying again...",
                        static_cast<int>(upload_status));
                    break;
                }
            }
            // otherwise in progress, so continue this loop
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

mavsdk::Telemetry::Position MavlinkClient::getPosition() const {
    return this->telemetry->position();
}

double MavlinkClient::getHeading() const {
    return this->telemetry->heading().heading_deg;
}

float MavlinkClient::getAirspeed() const {
    return this->telemetry->fixedwing_metrics().airspeed_m_s;
}

double MavlinkClient::getGroundspeed() const {
    const auto velocity_ned = this->telemetry->velocity_ned();
    return std::sqrt(std::pow(velocity_ned.east_m_s, 2) + std::pow(velocity_ned.north_m_s, 2));
}
