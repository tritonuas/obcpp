#include "network/mavlink.hpp"

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/param/param.h>
#include <mavsdk/plugins/geofence/geofence.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "core/mission_state.hpp"
#include "pathing/mission_path.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"

using namespace std::chrono_literals;  // NOLINT

MavlinkClient::MavlinkClient(OBCConfig config)
    : mavsdk(mavsdk::Mavsdk::Configuration(mavsdk::Mavsdk::ComponentType::CompanionComputer)) {
    std::string link = config.network.mavlink.connect;

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
        std::this_thread::sleep_for(5s);
    }

    // it wont have gotten a heartbeat immediately so just give it a moment so we don't
    // have to wait the 3s every time
    std::this_thread::sleep_for(100ms);
    // Wait for the system to connect via heartbeat
    while (mavsdk.systems().size() == 0) {
        LOG_F(WARNING, "No heartbeat. Trying again in 3 seconds...");
        std::this_thread::sleep_for(3s);
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
    this->param = std::make_unique<mavsdk::Param>(system);

    // iterate through all config parameters and upload to the plane
    for (const auto& [param, val] : config.mavlink_parameters.param_map) {
        LOG_F(INFO, "Setting %s to %d", param.c_str(), val);
        while (true) {
            // stupid hack: need to change config file to encode type of data as well, 
            // or figure that out in a smart way
            auto result = mavsdk::Param::Result::Unknown;
            if (param == "FS_LONG_TIMEOUT" ||
                param == "AFS_RC_FAIL_TIME" ||
                param == "FS_SHORT_TIMEOUT") {
                result = this->param->set_param_float(param, val);
            } else {
                result = this->param->set_param_int(param, val);
            }

            if (result != mavsdk::Param::Result::Success) {
                LOG_S(ERROR) << "Failed to set param " << result;
                std::this_thread::sleep_for(1s);
                continue;
            }
            LOG_F(INFO, "Successfully set param");
            break;
        }
    }

    /*
    LOG_F(INFO, "Logging out all mavlink params at TRACE level...");
    auto all_params = this->param->get_all_params();
    VLOG_S(TRACE) << all_params;
    */

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

        LOG_S(WARNING) << "Setting mavlink polling rate to 1.0 failed: " << set_rate_result
                       << ". Trying again...";
        std::this_thread::sleep_for(1s);
    }

    // set home position

    LOG_F(INFO, "Attempting to set home position");
    bool set_home = false;
    int attempts = 0;
    while (attempts < 3) {
        mavsdk::MavlinkPassthrough::CommandLong command;
        command.command = MAV_CMD_DO_SET_HOME;
        command.target_compid = this->passthrough->get_our_compid();
        command.target_sysid = this->passthrough->get_our_sysid();
        command.param1 = 0; // use specified location
        command.param2 = 0; // dont care about yaw, pitch, roll
        command.param3 = 0; // ..
        command.param4 = 0; // ..
        command.param5 = 38.315339; // currently hardcoded to competitions RTL position, this should be set in gcs eventually
        command.param6 = -76.548108;
        command.param7 = 0; // altitude

        auto result = this->passthrough->send_command_long(command);
      
        if (result == mavsdk::MavlinkPassthrough::Result::Success) {
            LOG_F(INFO, "Successfully uploaded home position");
            set_home = true;
            break;
        }

        LOG_S(WARNING) << "Could not set home position because " << result << ". Trying again...";
        attempts++;
        std::this_thread::sleep_for(100ms);
    }
    if (!set_home) {
        LOG_F(WARNING, "Could not set home position!");
    }

    LOG_F(INFO, "Setting mavlink telemetry subscriptions");
    // Set subscription functions to keep internal data struct up to date
    this->telemetry->subscribe_position([this](mavsdk::Telemetry::Position position) {
        VLOG_F(DEBUG, "Latitude: %f\tLongitude: %f\tAGL Alt: %f\tMSL Alt: %f)",
               position.latitude_deg, position.longitude_deg, position.relative_altitude_m,
               position.absolute_altitude_m);

        Lock lock(this->data_mut);
        this->data.altitude_agl_m = position.relative_altitude_m;
        this->data.altitude_msl_m = position.absolute_altitude_m;
        this->data.lat_deg = position.latitude_deg;
        this->data.lng_deg = position.longitude_deg;
    });
    this->telemetry->subscribe_flight_mode([this](mavsdk::Telemetry::FlightMode flight_mode) {
        std::ostringstream stream;
        stream << flight_mode;

        std::string flight_mode_str = stream.str();

        static std::string prev_flight_mode = "";

        Lock lock(this->data_mut);
        this->data.flight_mode = flight_mode;

        if (flight_mode_str != prev_flight_mode) {
            LOG_F(INFO, "Mav Flight Mode: %s", flight_mode_str.c_str());
            prev_flight_mode = flight_mode_str;
        }
    });
    this->telemetry->subscribe_fixedwing_metrics(
        [this](mavsdk::Telemetry::FixedwingMetrics fwmets) {
            VLOG_F(DEBUG, "Airspeed: %f", fwmets.airspeed_m_s);

            Lock lock(this->data_mut);
            this->data.airspeed_m_s = static_cast<double>(fwmets.airspeed_m_s);
        });
    this->telemetry->subscribe_velocity_ned([this](mavsdk::Telemetry::VelocityNed vned) {
        const double groundspeed_m_s =
            std::sqrt(std::pow(vned.east_m_s, 2) + std::pow(vned.north_m_s, 2));

        VLOG_F(DEBUG, "Groundspeed: %f", groundspeed_m_s);

        Lock lock(this->data_mut);
        this->data.groundspeed_m_s = groundspeed_m_s;
    });
    this->telemetry->subscribe_armed([this](bool armed) {
        VLOG_F(DEBUG, "Armed: %d", armed);
        Lock lock(this->data_mut);
        this->data.armed = armed;
    });

    this->passthrough->subscribe_message(WIND_COV, [this](const mavlink_message_t& message) {
        // auto payload = message.payload64;
        // LOG_F(INFO, "UNIX TIME: %lu", payload[0]);

        // /*
        //     NOT TESTED - don't actually know where the data is in thie uint64_t[]
        //     TODO - test on actual pixhawk to make sure that the data makes sense
        // */

        // this->data.wind.x = payload[1];
        // this->data.wind.y = payload[2];
        // this->data.wind.z = payload[3];

        this->data.wind.x = 0;
        this->data.wind.y = 0;
        this->data.wind.z = 0;
    });
    // this->telemetry->subscribe_attitude_euler(
    //     [this](mavsdk::Telemetry::EulerAngle attitude) {
    //         VLOG_F(DEBUG, "Yaw: %f, Pitch: %f, Roll: %f)",
    //             attitude.yaw_deg, attitude.pitch_deg, attitude.roll_deg);

    //         Lock lock(this->data_mut);
    //         this->data.yaw_deg = attitude.yaw_deg;
    //         this->data.pitch_deg = attitude.pitch_deg;
    //         this->data.roll_deg = attitude.roll_deg;
    //     });
}

bool MavlinkClient::uploadMissionUntilSuccess(std::shared_ptr<MissionState> state,
                                              bool upload_geofence,
                                              const MissionPath& path) const {
    if (upload_geofence) {
        if (!this->uploadGeofenceUntilSuccess(state)) {
            return false;
        }
    }
    if (path.get().size() > 0) {
        if (!this->uploadWaypointsUntilSuccess(state, path)) {
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
    if (state->getInitPath().get().empty()) {
        LOG_F(ERROR, "Upload failed - no initial path");
        return false;
    }

    // Parse the flight boundary / geofence information
    mavsdk::Geofence::Polygon flight_bound;
    flight_bound.fence_type = mavsdk::Geofence::FenceType::Inclusion;
    for (const auto& coord : mission->flightboundary()) {
        flight_bound.points.push_back(mavsdk::Geofence::Point{
            .latitude_deg{coord.latitude()},
            .longitude_deg{coord.longitude()},
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
                                                const MissionPath& waypoints) const {
    LOG_SCOPE_F(INFO, "Uploading waypoints");


    while (true) {
        LOG_F(INFO, "Sending waypoint information...");

        std::mutex resultMut;
        std::optional<mavsdk::MissionRaw::Result> result{};

        this->mission->upload_mission_async(
            waypoints.getCommands(), [&result, &resultMut](const mavsdk::MissionRaw::Result& res) {
                resultMut.lock();
                result = res;
                resultMut.unlock();
            });

        while (true) {
            // uh well you see
            resultMut.lock();
            if (result.has_value()) {
                resultMut.unlock();
                break;
            }
            resultMut.unlock();
        }

        if (result.value() == mavsdk::MissionRaw::Result::Success) {
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

double MavlinkClient::yaw_deg() {
    Lock lock(this->data_mut);
    return this->data.yaw_deg;
}

double MavlinkClient::pitch_deg() {
    Lock lock(this->data_mut);
    return this->data.pitch_deg;
}

double MavlinkClient::roll_deg() {
    Lock lock(this->data_mut);
    return this->data.roll_deg;
}

XYZCoord MavlinkClient::wind() {
    Lock lock(this->data_mut);
    return this->data.wind;
}

bool MavlinkClient::isArmed() {
    Lock lock(this->data_mut);
    return this->data.armed;
}

mavsdk::Telemetry::FlightMode MavlinkClient::flight_mode() {
    Lock lock(this->data_mut);
    return this->data.flight_mode;
}

int32_t MavlinkClient::curr_waypoint() const {
    return this->mission->mission_progress().current;
}

bool MavlinkClient::isMissionFinished() {
    // Boolean representing if mission is finished
    return this->mission->mission_progress().current == this->mission->mission_progress().total;
}

bool MavlinkClient::isAtFinalWaypoint() {
    return this->mission->mission_progress().current == this->mission->mission_progress().total - 1;
}

mavsdk::Telemetry::RcStatus MavlinkClient::get_conn_status() {
    return this->telemetry->rc_status();
}

/**
 * Goes through the sequence of checking vehicle health -> arm vehicle -> takeoff -> hover at set
 * altitude.
 */
bool MavlinkClient::armAndHover(std::shared_ptr<MissionState> state) {
    LOG_F(INFO, "Attempting to arm and hover");
    LOG_F(INFO, "Checking vehicle health...");
    // Vehicle can only be armed if status is healthy
    if (this->telemetry->health_all_ok() != true) {
        LOG_F(ERROR, "Vehicle not ready to arm");
        return false;
    }

    LOG_F(INFO, "Attempting to arm...");
    // Attempt to arm the vehicle
    const mavsdk::Action::Result arm_result = this->action->arm();
    if (arm_result != mavsdk::Action::Result::Success) {
        LOG_F(ERROR, "Arming failed");
        return false;
    }

    const float TAKEOFF_ALT = state->config.takeoff.altitude_m;
    // for some reason on the sitl sometimes it gets to right below the takeoff
    // alt but then never reaches the exact value, so this is a hack to tell it
    // to takeoff to 1m above where we actually want to takeoff so we reach
    // the non adjusted altitude
    const float TAKEOFF_ALT_ADJ = TAKEOFF_ALT + 1;
    LOG_F(INFO, "Setting takeoff altitude to %f", TAKEOFF_ALT_ADJ);
    auto r1 = this->action->set_takeoff_altitude(TAKEOFF_ALT_ADJ);
    if (r1 != mavsdk::Action::Result::Success) {
        LOG_S(ERROR) << "FAIL: could not set takeoff alt " << r1;
        return false;
    }
    auto [r2, alt_checked] = this->action->get_takeoff_altitude();
    LOG_S(INFO) << "Queried takeoff alt to be " << alt_checked << " with status " << r2;

    LOG_F(INFO, "Attempting to take off to %fm...", TAKEOFF_ALT);
    auto takeoff_result = this->action->takeoff();
    if (takeoff_result != mavsdk::Action::Result::Success) {
        LOG_F(ERROR, "Take off failed");
        return false;
    }

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
 * Starts the currently uploaded mavlink mission
 */
bool MavlinkClient::startMission() {
    LOG_F(INFO, "Sending start mission command");
    auto start_result = this->mission->start_mission();
    if (start_result != mavsdk::MissionRaw::Result::Success) {
        LOG_S(ERROR) << "FAIL: Mission could not start " << start_result;
        return false;
    }

    LOG_F(INFO, "Mission Started!");
    return true;
}

void MavlinkClient::KILL_THE_PLANE_DO_NOT_CALL_THIS_ACCIDENTALLY() {
    LOG_F(ERROR, "KILLING THE PLANE: SETTING AFS_TERMINATE TO 1");
    auto result = this->param->set_param_int("AFS_TERMINATE", 1);
    LOG_S(ERROR) << "KILL RESULT: " << result;
}
