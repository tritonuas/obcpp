#include "camera/interface.hpp"

ImageTelemetry::ImageTelemetry(double latitude, double longitude,
    double altitude, double airspeed, double yaw, double pitch, double roll) 
        : latitude(latitude), longitude(longitude), altitude(altitude), 
        airspeed(airspeed), yaw(yaw), pitch(pitch), roll(roll) {}


ImageData::ImageData(std::string NAME, std::string PATH, cv::Mat DATA, 
    ImageTelemetry TELEMETRY) 
        : NAME(NAME), PATH(PATH), DATA(DATA), TELEMETRY(TELEMETRY) {};

std::string ImageData::getName() {
    return NAME;

}
std::string ImageData::getPath() {
    return PATH;
}

cv::Mat ImageData::getData() {
    return DATA;
}

ImageTelemetry ImageData::getTelemetry() {
    return TELEMETRY;
}