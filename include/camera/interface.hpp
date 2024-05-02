#ifndef INCLUDE_CAMERA_INTERFACE_HPP_
#define INCLUDE_CAMERA_INTERFACE_HPP_

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <deque>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

#include "utilities/datatypes.hpp"

using json = nlohmann::json;
using Mat = cv::Mat;

// class to contain all telemetry that should be tagged with an image.
// In the future this could be in a mavlink file.
struct ImageTelemetry {
    ImageTelemetry(double latitude, double longitude, double altitude, double airspeed,
                   double heading, double yaw, double pitch, double roll);
    double latitude;
    double longitude;
    double altitude;
    double airspeed;
    double heading;
    double yaw;
    double pitch;
    double roll;
};

/*
 * FYI: this is class that will standardize image data but
 * if all of our cameras have a uniform image output type
 * this class is completely and utterly useless.
 *                                      - Boris
 *
 * We will also need to develop a custom converter
 *                                      - Boris (10/11)
 */
class ImageData {
 private:
    std::string NAME;
    std::string PATH;
    cv::Mat DATA;
    ImageTelemetry TELEMETRY;

 public:
    ImageData(std::string NAME, std::string PATH, cv::Mat DATA, ImageTelemetry TELEMETRY);
    ImageData(const ImageData&) = default;

    std::string getName() const;
    std::string getPath() const;
    cv::Mat getData() const;
    ImageTelemetry getTelemetry() const;
};


class CameraInterface {
 private:
    CameraConfig config;

 public:
    explicit CameraInterface(const CameraConfig& config);
    virtual ~CameraInterface() = default;

    virtual void connect() = 0;
    virtual bool isConnected() = 0;

    virtual void startTakingPictures(const std::chrono::milliseconds& interval) = 0;
    virtual void stopTakingPictures() = 0;

    virtual std::optional<ImageData> getLatestImage() = 0;
    virtual std::deque<ImageData> getAllImages() = 0;
};

#endif  // INCLUDE_CAMERA_INTERFACE_HPP_
