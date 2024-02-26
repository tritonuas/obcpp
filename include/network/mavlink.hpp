#ifndef INCLUDE_NETWORK_MAVLINK_HPP_
#define INCLUDE_NETWORK_MAVLINK_HPP_

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/geofence/geofence.h>
#include <memory>
#include <mutex>
#include <string>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <cmath>
#include <utility>


class MissionState;

class MavlinkClient {
 public:
    /*
     * Contructor for MavlinkClient
     * @param link link to the MAVLink connection ip port -> [protocol]://[ip]:[port]
     * example:
     *   MavlinkClient("tcp://192.168.65.254:5762")
     */ 
    explicit MavlinkClient(const char* link);

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
    bool uploadMissionUntilSuccess(std::shared_ptr<MissionState> state) const;

    std::pair<double, double> latlng_deg();
    double altitude_agl_m();
    double altitude_msl_m();
    double groundspeed_m_s();
    double airspeed_m_s();
    double heading_deg();
    mavsdk::Telemetry::FlightMode flight_mode();


    /**
     * Send a custom Mavlink command which is not implemented by mavsdk.
     * You have to check the mavlink documentation and supply all the arguments yourself.
     * WARNING: There are no compile time checks that you are passing in the right data!
     *          Calls to this MUST be carefully checked!
     */
    mavsdk::MavlinkPassthrough::Result sendCustomMavlinkCommand(uint8_t target_sysid, uint8_t target_compid, uint16_t command,
                                 float param1, float param2, float param3, float param4, float param5,
                                 float param6, float param7);

 private:
    mavsdk::Mavsdk mavsdk;
    std::shared_ptr<mavsdk::System> system;
    std::unique_ptr<mavsdk::Telemetry> telemetry;
    std::unique_ptr<mavsdk::Mission> mission;
    std::unique_ptr<mavsdk::Geofence> geofence;

    struct Data {
        double lat_deg {};
        double lng_deg {};
        double altitude_agl_m {};
        double altitude_msl_m {};
        double groundspeed_m_s {};
        double airspeed_m_s {};
        double heading_deg {};
        mavsdk::Telemetry::FlightMode flight_mode {};
    } data;
    std::mutex data_mut;
};

#endif  // INCLUDE_NETWORK_MAVLINK_HPP_
