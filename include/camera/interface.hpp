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

// struct to contain all telemetry that should be tagged with an image.
struct ImageTelemetry {
    double latitude;
    double longitude;
    double altitude;
    double airspeed;
    double heading;
    double yaw;
    double pitch;
    double roll;
};

struct ImageData {
    cv::Mat DATA;
    ImageTelemetry TELEMETRY;
};

class CameraInterface {
 protected:
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
