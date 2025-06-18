#include <chrono>
#include <thread>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <loguru.hpp>

#include "camera/picamera.hpp"
#include "camera/interface.hpp"
#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/common.hpp"

using json = nlohmann::json;


// struct CameraConfig {
//     // either "mock" or "lucid"
//     std::string type;
//     // directory to save images to
//     std::string save_dir;
//     // whether or not to save to save_dir
//     bool save_images_to_file;
//     struct {
//         // directory to randomly pick images from
//         // for the mock camera
//         std::string images_dir;
//     } mock;
// };

// Use the robust GStreamer pipeline that worked on the first run
std::string get_gstreamer_pipeline(int width, int height, int framerate) {
    return
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), width=(int)" + std::to_string(width) + ", height=(int)" + std::to_string(height) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) + "/1 ! "
        "queue ! "
        "nvvidconv ! "
        "video/x-raw, format=(string)BGRx ! "
        "queue ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! "
        "appsink drop=true";
}


PiCamera::PiCamera(CameraConfig config) : 
    CameraInterface(config),
    cap(get_gstreamer_pipeline(1920, 1080, 30), cv::CAP_GSTREAMER) 
{
    if (!this->cap.isOpened()) {
        LOG_F(FATAL, "FATAL: Could not open picamera");
    } else {
        LOG_F(INFO, "PiCamera opened successfully.");
    }
}


void PiCamera::connect() {

}

PiCamera::~PiCamera() {

}

bool PiCamera::isConnected() {
	 return true;
}

void PiCamera::startTakingPictures(const std::chrono::milliseconds& interval, 
    std::shared_ptr<MavlinkClient> mavlinkClient) {
    this->isTakingPictures = true;
    try {
        this->captureThread = std::thread(&PiCamera::captureEvery, this, interval, mavlinkClient);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void PiCamera::stopTakingPictures(){
    if (!this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = false;

    this->captureThread.join();
}

std::optional<ImageData> PiCamera::getLatestImage() {
    // Use an exclusive (write) lock because we are modifying the queue.
    WriteLock lock(this->imageQueueLock);
    if (this->imageQueue.empty()) {
        return std::nullopt;
    }
    ImageData lastImage = this->imageQueue.front();
    this->imageQueue.pop_front();
    return lastImage;
}

std::deque<ImageData> PiCamera::getAllImages() {
    // Use an exclusive (write) lock because we are clearing the queue.
    WriteLock lock(this->imageQueueLock);
    std::deque<ImageData> outputQueue;
    this->imageQueue.swap(outputQueue); // swap is an efficient way to move and clear
    return outputQueue;
}

std::optional<ImageData> PiCamera::takePicture(const std::chrono::milliseconds& timeout,
std::shared_ptr<MavlinkClient> mavlinkClient) {
    cv::Mat frame;

    if(!(this->cap.read(frame))) {
	LOG_F(ERROR, "ERROR: Failed to capture Picture/Frame");
    }

    uint64_t timestamp = getUnixTime_s().count();

    ImageData imageData {
        .DATA = frame,
        .TIMESTAMP = timestamp,
        .TELEMETRY = queryMavlinkImageTelemetry(mavlinkClient),
    };

    return imageData;
}

void PiCamera::startStreaming() {
    
}


void PiCamera::captureEvery(const std::chrono::milliseconds& interval, 
    std::shared_ptr<MavlinkClient> mavlinkClient) {
    
    loguru::set_thread_name("picamera");

    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with picam.");
        auto imageData = this->takePicture(interval, mavlinkClient);

        if (!imageData.has_value()) {
            LOG_F(WARNING, "Failed to take picture with picam");
            continue;
        }

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push_back(imageData.value());
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
}

