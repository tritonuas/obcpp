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

using json = nlohmann::json;
using Mat = cv::Mat;

// struct to contain all telemetry that should be tagged with an image.
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

/**
 * Given a shared ptr to a mavlink client, query it for the telemetry
 * information needed for ImageTelemetry and the CV pipeline.
 * 
 * If nullptr is passed for mavlinkClient, nullopt is returned 
*/
std::optional<ImageTelemetry> queryMavlinkImageTelemetry(
    std::shared_ptr<MavlinkClient> mavlinkClient);

struct ImageData {
    cv::Mat DATA;
    uint64_t TIMESTAMP;
    std::optional<ImageTelemetry> TELEMETRY;
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
    virtual ~CameraInterface() = default;

    virtual void connect() = 0;
    virtual bool isConnected() = 0;

    /**
     * Start taking photos at an interval in a background thread.
     * Also requires a shared_ptr to a MavlinkClient to tag 
     * images with flight telemetry at capture time.
    */
    virtual void startTakingPictures(const std::chrono::milliseconds& interval,
    std::shared_ptr<MavlinkClient> mavlinkClient) = 0;
    /**
     * Close background thread started by startTakingPictures
    */
    virtual void stopTakingPictures() = 0;

    // Get the latest buffered image
    virtual std::optional<ImageData> getLatestImage() = 0;
    // Get all the recently buffered images
    virtual std::deque<ImageData> getAllImages() = 0;

    virtual void startStreaming() = 0;

    /**
    * Blocking call that takes an image. If it takes longer than the timeout 
    * to capture the image, no image is returned.
    */
    virtual std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout,
        std::shared_ptr<MavlinkClient> mavlinkClient) = 0;
};

#endif  // INCLUDE_CAMERA_INTERFACE_HPP_
