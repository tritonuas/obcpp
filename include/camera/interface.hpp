#ifndef INCLUDE_CAMERA_INTERFACE_HPP_
#define INCLUDE_CAMERA_INTERFACE_HPP_

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <deque>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

#include "network/mavlink.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"

using json = nlohmann::json;
using Mat = cv::Mat;

struct ImageTelemetry {
    double latitude_deg;
    double longitude_deg;
    double altitude_agl_m;
    double airspeed_m_s;
    double heading_deg;
    double yaw_deg;
    double pitch_deg;
    double roll_deg;
};

struct ImageData {
    cv::Mat DATA;
    uint64_t TIMESTAMP;
    std::optional<ImageTelemetry> TELEMETRY;

    /**
     * Saves the image to a file
     * @param directory directory to save the image in
     * @returns true/false if saving was successful
     */
    bool saveToFile(std::string directory) const;
};

std::string cvMatToBase64(cv::Mat image);

void saveImageToFile(cv::Mat image, const std::filesystem::path& filepath);

void saveImageTelemetryToFile(const ImageTelemetry& telemetry,
                              const std::filesystem::path& filepath);

class CameraInterface {
    protected:
       CameraConfig config;
   
    public:
       explicit CameraInterface(const CameraConfig& config);
    // explicit CameraInterface();
       virtual ~CameraInterface() = default;
   
       virtual void connect() = 0;

       virtual std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout) = 0;
   };

#endif  // INCLUDE_CAMERA_INTERFACE_HPP_
