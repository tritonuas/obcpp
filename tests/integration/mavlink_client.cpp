#include "network/mavlink.hpp"
#include "utilities/logging.hpp"

/*
 * Test mavsdk connection to an endpoint 
 * bin/mavsdk [connection_link]
 * connection_link -> [protocol]://[ip]:[port]
 * example: 
 *   bin/mavlink_client tcp://127.0.0.1:5760
 *   bin/mavlink_client serial:///dev/ttyACM0
 *  
 *  Note: if you are trying to test connection with serial from inside the dev container,
 *  you have to follow these steps:
 * 
 *  1. add "runArgs": ["--device=/dev/ttyACM0"] to the devcontainer.json 
 *     file and rebuild the container
 *  2. in the terminal run `sudo usermod -aG dialout $USER`
 *  3. in the terminal run `newgrp dialout`
 *  
 *  This should fuck up your terminal but allow you to use the ttyACM0 serial port 
 *  from inside the container. YIPIEEE!
 */ 
int main(int argc, char *argv[]) {

    if (argc != 2) {
        LOG_F(ERROR, "Expected use: bin/mavsdk [connection]");
        return 1;
    }

    LOG_S(INFO) << "Attempting to connect at " << argv[1];
    
    MavlinkClient mav(argv[1]);

    while (true) {
        LOG_S(INFO) << "Groundspeed: " << mav.groundspeed_m_s();
        LOG_S(INFO) << "Airspeed: " << mav.airspeed_m_s();
        LOG_S(INFO) << "Flight Mode: " << mav.flight_mode();
        LOG_S(INFO) << "LatLng: " << mav.latlng_deg().first << 
            ", " << mav.latlng_deg().second;
        LOG_S(INFO) << "Heading: " << mav.heading_deg();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}