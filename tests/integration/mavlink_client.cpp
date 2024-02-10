#include <iostream>
#include <network/mavlink.hpp>


/*
* Test mavsdk connection to an endpoint 
* bin/mavsdk [connection_link]
* connection_link -> [protocol]://[ip]:[port]
* example: 
*   bin/mavsdk tcp://127.0.0.1:5760
*/ 
int main(int argc, char *argv[]) {
    
    mavlink::MavlinkClient mavlinkClient("tcp://192.168.65.254:5762");
    mavlink::Position position = mavlinkClient.getPosition();
    std::cout<<"Latitude: "<<position.latitudeDeg<<std::endl;
    std::cout<<"Longitude: "<<position.longitudeDeg<<std::endl;
    std::cout<<"Absolute Altitude: "<<position.absoluteAltitudeM<<std::endl;
    std::cout<<"Relative Altitude: "<<position.relativeAltitudeM<<std::endl;
    // mavlink::Battery battery = mavlinkClient.getBattery();
    // std::cout<<"Battery Voltage: "<<battery.voltage<<std::endl;
    // std::cout<<"Battery Percent: "<<battery.percent<<std::endl;
    mavlink::Heading heading = mavlinkClient.getHeading();
    std::cout<<"Heading Degree: "<<heading.headingDeg<<std::endl;

    
    mavlink::FixedwingMetrics fixedwingMetrics = mavlinkClient.getFixedwingMetrics();
    std::cout<<"Airspeed: "<<fixedwingMetrics.airspeed<<std::endl;

    mavlink::GroundSpeed groundSpeed = mavlinkClient.getGroundSpeed();
    std::cout<<"Ground Speed: "<<groundSpeed.groundSpeed<<std::endl;

    return 0;
}