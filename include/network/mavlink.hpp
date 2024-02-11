#ifndef INCLUDE_NETWORK_MAVLINK_HPP_
#define INCLUDE_NETWORK_MAVLINK_HPP_

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/geofence/geofence.h>
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <cmath>

#include "core/mission_config.hpp"

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
     */
    void uploadMissionUntilSuccess(MissionConfig& mission) const;

    /*
     * Get the plane's location information, which includes lat (deg), lng (deg),
     * relative alt (m), and absolute alt (m)
     */
    mavsdk::Telemetry::Position getPosition() const;

    /*
     * Gets the plane's heading in degrees
     */
    double getHeading() const;

    /*
     * Gets the plane's airspeed in meters per second
     */
    float getAirspeed() const;

    /*
     * Gets the plane's groundspeed in meters per second
     */
    double getGroundspeed() const;

 private:
    mavsdk::Mavsdk mavsdk;
    std::shared_ptr<mavsdk::System> system;
    std::unique_ptr<mavsdk::Telemetry> telemetry;
    std::unique_ptr<mavsdk::Mission> mission;
    std::unique_ptr<mavsdk::Geofence> geofence;

    // TODO: set as config in obc config file
    static constexpr const double TELEMETRY_UPDATE_RATE = 1.0;
};

#endif  // INCLUDE_NETWORK_MAVLINK_HPP_
