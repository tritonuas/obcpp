#include "network/mavlink.hpp"

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
            /*
            * Contructor for MavlinkClient
            * @param link link to the MAVLink connection ip port -> [protocol]://[ip]:[port]
            * example:
            *   MavlinkClient("tcp://192.168.65.254:5762")
            */ 
            MavlinkClient(std::string const link) {
                std::cout<<link<<std::endl;
                mavsdk::ConnectionResult conn_result = this->mavsdk.add_any_connection(link);
                if (conn_result == mavsdk::ConnectionResult::Success) { 
                    std::cout<<"Connected to: "<<link<<std::endl;
                }
                // Wait for the system to connect via heartbeat
                while (mavsdk.systems().size() == 0) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    std::cout<<"Finding system..."<<std::endl;
                }
                // System got discovered.
                this->system = mavsdk.systems()[0];

                // Create instance of Telemetry and Mission
                this->telemetry = std::make_unique<mavsdk::Telemetry>(system);
                this->mission = std::make_unique<mavsdk::Mission>(system);

                // Set position update rate (1 Hz)
                const mavsdk::Telemetry::Result set_rate_result = this->telemetry->set_rate_position(1.0);
                if (set_rate_result != mavsdk::Telemetry::Result::Success) {
                    // handle rate-setting failure (in this case print error)
                    std::cout << "Setting rate failed:" << set_rate_result << '\n';
                }
            }

            Position getPosition() {
                Position position;
                mavsdk::Telemetry::Position telemetryPosition = this->telemetry->position();
                position.latitudeDeg = telemetryPosition.latitude_deg;
                position.longitudeDeg = telemetryPosition.longitude_deg;
                position.absoluteAltitudeM = telemetryPosition.absolute_altitude_m;
                position.relativeAltitudeM = telemetryPosition.relative_altitude_m;
                return position;
            }

            Heading getHeading() {
                Heading heading;
                mavsdk::Telemetry::Heading telemetryHeading = this->telemetry->heading();
                heading.headingDeg = telemetryHeading.heading_deg;
                return heading;
            }

            Battery getBattery() {
                Battery battery;
                mavsdk::Telemetry::Battery telemetryBattery = this->telemetry->battery();
                battery.voltage = telemetryBattery.voltage_v;
                battery.percent = telemetryBattery.remaining_percent;
                return battery;
            }

            FixedwingMetrics getFixedwingMetrics() {
                FixedwingMetrics fixedwingMetrics;
                mavsdk::Telemetry::FixedwingMetrics telemetryFixedwingMetrics = this->telemetry->fixedwing_metrics();
                fixedwingMetrics.airspeed = telemetryFixedwingMetrics.airspeed_m_s;
                return fixedwingMetrics;
            }

            GroundSpeed getGroundSpeed() {
                GroundSpeed groundSpeed;
                mavsdk::Telemetry::VelocityNed telemetryVelocityNed = this->telemetry->velocity_ned();
                groundSpeed.northSpeed = telemetryVelocityNed.north_m_s;
                groundSpeed.eastSpeed = telemetryVelocityNed.east_m_s;
                groundSpeed.groundSpeed = sqrt(pow(telemetryVelocityNed.east_m_s, 2) + pow(telemetryVelocityNed.north_m_s, 2));
                return groundSpeed;
            }
    };
}

#endif // NETWORK_MAVLINK_HPP_ 