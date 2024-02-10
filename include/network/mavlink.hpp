#ifndef NETWORK_MAVLINK_HPP_ 
#define NETWORK_MAVLINK_HPP_ 
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include <optional>
#include <cmath>


namespace mavlink {
    struct Position {
        double latitudeDeg;
        double longitudeDeg;
        float absoluteAltitudeM;
        float relativeAltitudeM;
    };

    struct Battery {
        uint32_t id;
        float voltage; // Voltage (unit: volts)
        float percent; // Estimated battery remaining (range: 0.0~1.0)
    };

    struct Heading {
        double headingDeg;
    };
    
    struct FixedwingMetrics {
        float airspeed;
    };

    struct GroundSpeed {
        float northSpeed;
        float eastSpeed;
        float groundSpeed;
    };


    class MavlinkClient {
        mavsdk::Mavsdk mavsdk;
        std::shared_ptr<mavsdk::System> system;
        std::unique_ptr<mavsdk::Telemetry> telemetry;
        std::unique_ptr<mavsdk::Mission> mission;
        public:
            MavlinkClient(std::string const link);

            Position getPosition();

            Heading getHeading();

            Battery getBattery();

            FixedwingMetrics getFixedwingMetrics();

            GroundSpeed getGroundSpeed();
    };
}

#endif // NETWORK_MAVLINK_HPP_ 