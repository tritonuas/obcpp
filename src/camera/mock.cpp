#include "camera/mock.hpp"

#include <chrono>
#include <thread>
#include <optional>
#include <deque>
#include <filesystem>

#include <loguru.hpp>

#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/rng.hpp"


MockCamera::MockCamera(CameraConfig config) : CameraInterface(config) {
    std::ranges::for_each(
        std::filesystem::directory_iterator{this->config.mock.images_dir},
        [this](const auto& dir_entry) {
            cv::Mat img = cv::imread(dir_entry.path().string());
            // if the image is read
            if (img.data != NULL) {
                this->mock_images.push_back(img);
            }
        });
}

MockCamera::~MockCamera() {
    this->stopTakingPictures();
}

void MockCamera::connect() { return; }

bool MockCamera::isConnected() { return true; }

void MockCamera::startTakingPictures(const std::chrono::milliseconds& interval,
    std::shared_ptr<MavlinkClient> mavlinkClient) {
    this->isTakingPictures = true;
    try {
        this->captureThread = std::thread(&MockCamera::captureEvery, this, interval, mavlinkClient);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MockCamera::stopTakingPictures() {
    if (!this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = false;

    this->captureThread.join();
}

std::optional<ImageData> MockCamera::getLatestImage() {
    ReadLock lock(this->imageQueueLock);
    ImageData lastImage = this->imageQueue.front();
    this->imageQueue.pop_front();
    return lastImage;
}

std::deque<ImageData> MockCamera::getAllImages() {
    ReadLock lock(this->imageQueueLock);
    std::deque<ImageData> outputQueue = this->imageQueue;
    this->imageQueue = std::deque<ImageData>();
    return outputQueue;
}

void MockCamera::captureEvery(const std::chrono::milliseconds& interval,
    std::shared_ptr<MavlinkClient> mavlinkClient) {
    loguru::set_thread_name("mock camera");
    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with mock camera. Using images from %s",
            this->config.mock.images_dir.c_str());
        
        auto imageData = this->takePicture(interval, mavlinkClient);


        if (!imageData.has_value()){
            LOG_F(WARNING, "Failed to take picture with mock camera");
            continue;
        }

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push_back(imageData.value();
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
}

std::optional<ImageData> MockCamera::takePicture(const std::chrono::milliseconds& timeout,
        std::shared_ptr<MavlinkClient> mavlinkClient) {
    int random_idx = randomInt(0, this->mock_images.size()-1);

    
    std::optional<ImageTelemetry> telemetry = queryMavlinkImageTelemetry(mavlinkClient);
    cv:Mat newImage = this->mock_images.at(random_idx);

    ImageData imageData {
        .DATA = newImage,
        .TELEMETRY = telemetry,
    };

    return imageData;
}

void MockCamera::startStreaming(){}

