#include "network/mavlink.hpp"

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/geofence/geofence.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

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

using namespace std::chrono_literals; // NOLINT

MavlinkClient::MavlinkClient(std::string link):
    mavsdk(mavsdk::Mavsdk::Configuration(mavsdk::Mavsdk::ComponentType::CompanionComputer)) {
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
    this->mission = std::make_unique<mavsdk::MissionRaw>(system);
    this->geofence = std::make_unique<mavsdk::Geofence>(system);
    this->action = std::make_unique<mavsdk::Action>(system);
    this->passthrough = std::make_unique<mavsdk::MavlinkPassthrough>(system);

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
    this->telemetry->subscribe_armed(
        [this](bool armed) {
            VLOG_F(DEBUG, "Armed: %d", armed);
            Lock lock(this->data_mut);
            this->data.armed = armed;
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
    flight_bound.fence_type = mavsdk::Geofence::FenceType::Inclusion;
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
        mavsdk::Geofence::GeofenceData geofence_data;
        geofence_data.polygons = {flight_bound};
        auto geofence_result = this->geofence->upload_geofence(geofence_data);
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
    std::vector<mavsdk::MissionRaw::MissionItem> mission_items;
    int i = 0;
    for (const auto& coord : waypoints) {
        mavsdk::MissionRaw::MissionItem new_raw_item_nav {};
        new_raw_item_nav.seq = i;
        new_raw_item_nav.frame = 3;  // MAV_FRAME_GLOBAL_RELATIVE_ALT
        new_raw_item_nav.command = 16;  // MAV_CMD_NAV_WAYPOINT
        new_raw_item_nav.current = (i == 0) ? 1 : 0;
        new_raw_item_nav.autocontinue = 1;
        new_raw_item_nav.param1 = 0.0;  // Hold
        new_raw_item_nav.param2 = 7.0;  // Accept Radius 7.0m close to 25ft
        new_raw_item_nav.param3 = 0.0;  // Pass Radius
        new_raw_item_nav.param4 = NAN;  // Yaw
        new_raw_item_nav.x = int32_t(std::round(coord.latitude() * 1e7));
        new_raw_item_nav.y = int32_t(std::round(coord.longitude() * 1e7));
        new_raw_item_nav.z = coord.altitude();
        new_raw_item_nav.mission_type = 0;  // MAV_MISSION_TYPE_MISSION
        mission_items.push_back(new_raw_item_nav);
        i++;
    }

    while (true) {
        LOG_F(INFO, "Sending waypoint information...");

        std::optional<mavsdk::MissionRaw::Result> result {};

        this->mission->upload_mission_async(mission_items,
            [&result](const mavsdk::MissionRaw::Result& res) {
                result = res;
            });

        while (!result.has_value()) {}

        if (result == mavsdk::MissionRaw::Result::Success) {
            LOG_F(INFO, "Successfully uploaded mission");
            break;
        } else {
            LOG_S(ERROR) << "Error uploading mission: " << result.value() << ". Trying again.";
        }

        std::this_thread::sleep_for(500ms);
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

bool MavlinkClient::isArmed() {
    Lock lock(this->data_mut);
    return this->data.armed;
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
    return this->mission->mission_progress().current == this->mission->mission_progress().total;
}

mavsdk::Telemetry::RcStatus MavlinkClient::get_conn_status() {
    return this->telemetry->rc_status();
}

/**
 * Goes through the sequence of checking vehicle health -> arm vehicle -> takeoff -> hover at set altitude.
*/
bool MavlinkClient::armAndHover(){
    LOG_F(INFO, "Attempting to arm and hover");
    LOG_F(INFO, "Checking vehicle health...");
    //Vehicle can only be armed if status is healthy
    if(this->telemetry->health_all_ok() != true){
        LOG_F(ERROR, "Vehicle not ready to arm");
        return false;
    }

    LOG_F(INFO, "Attempting to arm...");
    //Attemp to arm the vehicle
    const mavsdk::Action::Result arm_result = this->action->arm();
    if(arm_result != mavsdk::Action::Result::Success){
        LOG_F(ERROR, "Arming failed");
        return false;
    }


    // TODO: config option for this
    const float TAKEOFF_ALT = 30.0f;
    auto r1 = this->action->set_takeoff_altitude(TAKEOFF_ALT);
    if (r1 != mavsdk::Action::Result::Success) {
        LOG_S(ERROR) << "FAIL: could not set takeoff alt " << r1;
        return false;
    }
    auto [r2, alt_checked ]= this->action->get_takeoff_altitude();
    LOG_S(INFO) << "Queried takeoff alt to be " << alt_checked << " with status " << r2;

    LOG_F(INFO, "Attempting to take off to %fm...", TAKEOFF_ALT);
    const mavsdk::Action::Result takeoff_result = this->action->takeoff();
    if (takeoff_result != mavsdk::Action::Result::Success) {
        LOG_F(ERROR, "Take off failed");
        return false;
    }

    // need to figure out correct values to send for a VTOL takeoff command
    // auto result = this->passthrough->send_command_int(mavsdk::MavlinkPassthrough::CommandInt {
    //     .target_sysid = this->passthrough->get_our_sysid(),
    //     .target_compid = this->passthrough->get_our_compid(),
    //     .command = MAV_CMD_NAV_VTOL_TAKEOFF,
    //     .frame = MAV_FRAME_GLOBAL_RELATIVE_ALT,
    //     .param1 = 0,
    //     .param2 = VTOL_TRANSITION_HEADING_NEXT_WAYPOINT, // unsure if arduplane will respect this
    //     .param3 = 0,
    //     .param4 = NAN,
    //     .x = 0,
    //     .y = 0,
    //     .z = TAKEOFF_ALT // currently hard coded to 30m (~100ft)
    // });
    // if (result != mavsdk::MavlinkPassthrough::Result::Success) {
    //     LOG_S(ERROR) << "FAIL: takeoff cmd not accepted because " << result;
    //     // return false;
    // }

    LOG_F(INFO, "Waiting to reach target altitude of %f", TAKEOFF_ALT);
    float current_position = 0;
    while (current_position < TAKEOFF_ALT) {
        current_position = this->telemetry->position().relative_altitude_m;
        LOG_F(INFO, "At alt %f", current_position);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    LOG_F(INFO, "Take off success");
    return true;
}

/**
 * Starts the mavlink mission and transitions vehicle from VTOL to fix wing.
*/
bool MavlinkClient::startMission() {
    LOG_F(INFO, "Attempting to start mission...");
    LOG_F(INFO, "Querying target takeoff altitude");
    const auto& [takeoff_result, target_alt] = this->action->get_takeoff_altitude();
    if (takeoff_result != mavsdk::Action::Result::Success) {
        LOG_S(ERROR) << "FAIL: could not query takeoff altitude: " << target_alt;
        return false;
    }
    LOG_F(INFO, "Target takeoff altitude is %f", target_alt);

    float current_position = this->telemetry->position().relative_altitude_m;

    LOG_F(INFO, "Checking target altitude");
    if(current_position < target_alt){
        LOG_F(ERROR, "FAIL: Vehicle has not reached desired altitude (%f < %f)", current_position, target_alt);
        return false;
    }

    LOG_F(INFO, "About to send start command");
    auto start_result = this->mission->start_mission();
    if (start_result != mavsdk::MissionRaw::Result::Success) {
        LOG_S(ERROR) << "FAIL: Mission could not start " << start_result;
        return false;
    }

    LOG_F(INFO, "About to transition to forward flight");
    auto fw_result = this->action->transition_to_fixedwing();
    if (fw_result != mavsdk::Action::Result::Success) {
        LOG_S(ERROR) << "FAIL: Transition to fix wing " << fw_result;
        return false;
    }

    LOG_F(INFO, "Mission Started!");
    return true;
}
