#ifndef INCLUDE_NETWORK_MAVLINK_HPP_
#define INCLUDE_NETWORK_MAVLINK_HPP_

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/geofence/geofence.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>
#include <mavsdk/plugins/param/param.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "pathing/mission_path.hpp"
#include "protos/obc.pb.h"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"

class MissionState;

class MavlinkClient {
 public:
    /*
     * Contructor for MavlinkClient
     * @param link link to the MAVLink connection ip port -> [protocol]://[ip]:[port]
     * example:
     *   MavlinkClient("tcp://192.168.65.254:5762")
     */
    explicit MavlinkClient(OBCConfig config);

    /*
     * BLOCKING. Continues to try to upload the mission based on the passed through MissionConfig
     * until it is successfully received by the plane.
     *
     * Uploading the mission takes two steps:
     * 1. uploading the geofence data (the flight boundary)
     * 2. uploading the waypoints data (this is what actually is called the "mission" in
     *    the mavlink terms)
     *
     * This function will continue to try and upload these two items until it has successfully
     * uploaded both. Since it is blocking, this should usually be called inside of a separate
     * thread if async behavior is desired. TODO: consider if it would be better to have this
     * function only attempt one mission upload, and have the retrying behavior start from the
     * outside.
     *
     * The only way this function fails is if there is no cached mission inside
     * of the state, or if the initial path is empty, which will make it return false. This
     * should never happen due to how the state machine is set up, but it is there just in case.
     */
    bool uploadMissionUntilSuccess(std::shared_ptr<MissionState> state, bool upload_geofence,
                                   const MissionPath& waypoints) const;

    bool uploadGeofenceUntilSuccess(std::shared_ptr<MissionState> state) const;
    bool uploadWaypointsUntilSuccess(std::shared_ptr<MissionState> state,
                                     const MissionPath& waypoints) const;

    std::pair<double, double> latlng_deg();
    double altitude_agl_m();
    double altitude_msl_m();
    double groundspeed_m_s();
    double airspeed_m_s();
    double heading_deg();
    double yaw_deg();
    double pitch_deg();
    double roll_deg();
    XYZCoord wind();
    bool isArmed();
    mavsdk::Telemetry::FlightMode flight_mode();
    int32_t curr_waypoint() const;
    bool isMissionFinished();
    bool isAtFinalWaypoint();
    bool setMissionItem(int item);
    size_t totalWaypoints();
    mavsdk::Telemetry::RcStatus get_conn_status();
    bool armAndHover(std::shared_ptr<MissionState> state);
    bool startMission();

    /*
     * Triggers a relay on the ArduPilot
     * @param relay_number The relay number to trigger (0-based, so RELAY2 = 1)
     * @param state True to turn on, false to turn off
     * @return True if successful, false otherwise
     */
    bool triggerRelay(int relay_number, bool state);

    void KILL_THE_PLANE_DO_NOT_CALL_THIS_ACCIDENTALLY();

 private:
    mavsdk::Mavsdk mavsdk;
    std::shared_ptr<mavsdk::System> system;
    std::unique_ptr<mavsdk::Telemetry> telemetry;
    std::unique_ptr<mavsdk::MissionRaw> mission;
    std::unique_ptr<mavsdk::Geofence> geofence;
    std::unique_ptr<mavsdk::Action> action;
    std::unique_ptr<mavsdk::MavlinkPassthrough> passthrough;
    std::unique_ptr<mavsdk::Param> param;

    struct Data {
        double lat_deg{};
        double lng_deg{};
        double altitude_agl_m{};
        double altitude_msl_m{};
        double groundspeed_m_s{};
        double airspeed_m_s{};
        double heading_deg{};
        double yaw_deg{};
        double pitch_deg{};
        double roll_deg{};
        XYZCoord wind{0, 0, 0};
        mavsdk::Telemetry::FlightMode flight_mode{};
        bool armed{};
    } data;
    std::mutex data_mut;
};

#endif  // INCLUDE_NETWORK_MAVLINK_HPP_
