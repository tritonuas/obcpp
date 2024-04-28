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
#include "mavlink.hpp"

MavlinkClient::MavlinkClient(std::string link) {
    LOG_F(INFO, "Connecting to Mav at %s", link.c_str());

    while (true) {
        LOG_F(INFO, "Attempting to add mav connection...");
        const auto conn_result = this->mavsdk.add_any_connection(link);
        if (conn_result == mavsdk::ConnectionResult::Success) {
            LOG_F(INFO, "Mavlink connection successfully established at %s", link.c_str());
            break;
        }

        LOG_S(WARNING) << "Mavlink connection failed: " << conn_result
            << ". Trying again in 5 seconds...";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Wait for the system to connect via heartbeat
    while (mavsdk.systems().size() == 0) {
        LOG_F(WARNING, "No heartbeat. Trying again in 3 seconds...");
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    LOG_F(INFO, "Mavlink heartbeat received");

    // System got discovered.
    this->system = mavsdk.systems()[0];

    // Create instance of Telemetry and Mission
    this->telemetry = std::make_unique<mavsdk::Telemetry>(system);
    this->mission = std::make_unique<mavsdk::Mission>(system);
    this->geofence = std::make_unique<mavsdk::Geofence>(system);
    this->action = std::make_unique<mavsdk::Action>(system);

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

bool MavlinkClient::uploadMissionUntilSuccess(std::shared_ptr<MissionState> state,
    bool upload_geofence, std::vector<GPSCoord> waypoints) const {
    if (upload_geofence) {
        if (!this->uploadGeofenceUntilSuccess(state)) {
            return false;
        }
    }
    if (waypoints.size() > 0) {
        if (!this->uploadWaypointsUntilSuccess(state, waypoints)) {
            return false;
        }
    }
    return true;
}

bool MavlinkClient::uploadGeofenceUntilSuccess(std::shared_ptr<MissionState> state) const {
    LOG_SCOPE_F(INFO, "Uploading Geofence");

    // Make sure everything is set up
    auto mission = state->mission_params.getCachedMission();
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

    return true;
}

bool MavlinkClient::uploadWaypointsUntilSuccess(std::shared_ptr<MissionState> state,
    std::vector<GPSCoord> waypoints) const {
    LOG_SCOPE_F(INFO, "Uploading waypoints");

    // Parse the waypoint information
    std::vector<mavsdk::Mission::MissionItem> mission_items;
    for (const auto& coord : waypoints) {
        mavsdk::Mission::MissionItem new_item {};
        mission_items.push_back(mavsdk::Mission::MissionItem {
            .latitude_deg {coord.latitude()},
            .longitude_deg {coord.longitude()},
            .relative_altitude_m {static_cast<float>(coord.altitude())},
            .is_fly_through {true}
        });
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

    return true;
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

/**
 * Helper function that goes along with isPointInPolygon().
*/
double MavlinkClient::angle2D(double x1, double y1, double x2, double y2) {
    double dtheta, theta1, theta2;

    theta1 = atan2(y1, x1);
    theta2 = atan2(y2, x2);
    dtheta = theta2 - theta1;

    while(dtheta > M_PI){
        dtheta -= 2*M_PI;
    }
    while(dtheta < -M_PI){
        dtheta += 2*M_PI;
    }

    return(dtheta);
}

/**
 * This function checks if a coordiante is inside a polygon region.
 * Source: https://stackoverflow.com/questions/4287780/detecting-whether-a-gps-coordinate-falls-within-a-polygon-on-a-map
*/
bool MavlinkClient::isPointInPolygon(std::pair<double, double> latlng, std::vector<XYZCoord> region){
    int n = region.size();
    double angle = 0;

    for(int i = 0; i < n; i++){
        double point_1_lat = region[i].x - latlng.first;
        double point_1_lng = region[i].y - latlng.second;
        double point_2_lat = region[(i+1)%n].x - latlng.first;
        double point_2_lng = region[(i+1)%n].y - latlng.second;
        angle += MavlinkClient::angle2D(point_1_lat, point_1_lng, point_2_lat, point_2_lng);
    }

    if(std::abs(angle) < M_PI){
        return false;
    }
    
    return true;
}

bool MavlinkClient::isMissionFinished(){
    //Boolean representing if mission is finished
    return this->mission->is_mission_finished().second;
}

mavsdk::Telemetry::RcStatus MavlinkClient::get_conn_status() {
    return this->telemetry->rc_status();
}

std::pair<std::string, bool> MavlinkClient::armAndHover(){

    //Vehicle can only be armed if status is healthy
    if(this->telemetry.health_all_ok != true){
        return ("Vehicle not ready to arm", false);
    }

    //Attemp to arm the vehicle
    const Action::Result arm_result = this->action.arm();
    if(arm_result != Action::Result::Success){
        return ("Arming failed:" + arm_result, false);
    }

    //Attemp to rise to takeoff altitude
    const Action::Result takeoff_result = this->action.takeoff();
    if (takeoff_result != Action::Result::Success) {
        return ("Take off failed: " + takeoff_result, false)
    }

    //A check to see if vehicle has reach takeoff altitude
    float target_alt = action.get_takeoff_altitude_m();
    float current_position = 0;
    while (current_position<target_alt) {
        current_position = this -> telemetry.position().relative_altitude_m;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return ("Success", true);
}
