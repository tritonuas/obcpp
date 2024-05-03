#include "camera/interface.hpp"

ImageTelemetry::ImageTelemetry(double latitude, double longitude, double altitude,
                               double airspeed, double heading,
                               double yaw, double pitch, double roll)
    : latitude(latitude),
      longitude(longitude),
      altitude(altitude),
      airspeed(airspeed),
      heading(heading),
      yaw(yaw),
      pitch(pitch),
      roll(roll) {}

ImageData::ImageData(std::string NAME, std::string PATH, cv::Mat DATA, ImageTelemetry TELEMETRY)
    : NAME(NAME), PATH(PATH), DATA(DATA), TELEMETRY(TELEMETRY) {}

std::string ImageData::getName() const { return NAME; }
std::string ImageData::getPath() const { return PATH; }

cv::Mat ImageData::getData() const { return DATA; }

ImageTelemetry ImageData::getTelemetry() const { return TELEMETRY; }

CameraConfiguration::CameraConfiguration(nlohmann::json config) : configJson(config) {}

CameraInterface::CameraInterface(CameraConfiguration config) : config(config) {}
