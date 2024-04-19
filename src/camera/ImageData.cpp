#include "camera/interface.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

ImageData::ImageData()
{
}

ImageData::ImageData(std::string NAME, std::string PATH, Mat DATA)
{
    this->NAME = NAME;
    this->PATH = PATH;
    this->DATA = DATA;
}

std::string ImageData::getName()
{
    return this->NAME;
}

std::string ImageData::getPath()
{
    return this->PATH;
}

cv::Mat ImageData::getData()
{
    return this->DATA;
}

ImageTelemetry::ImageTelemetry(double latitude, double longitude, double altitude, double airspeed,
                               double yaw, double pitch, double roll)
    : latitude(latitude),
      longitude(longitude),
      altitude(altitude),
      airspeed(airspeed),
      yaw(yaw),
      pitch(pitch),
      roll(roll) {}

ImageData::ImageData(std::string NAME, std::string PATH, cv::Mat DATA, ImageTelemetry TELEMETRY)
    : NAME(NAME), PATH(PATH), DATA(DATA), TELEMETRY(TELEMETRY) {}