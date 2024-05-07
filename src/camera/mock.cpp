#include "camera/mock.hpp"

#include <chrono>
#include <thread>
#include <optional>
#include <deque>
#include <filesystem>

#include <loguru.hpp>

#include "utilities/locks.hpp"
#include "utilities/rng.hpp"


MockCamera::MockCamera(CameraConfig config) : CameraInterface(config) {
    std::ranges::for_each(
        std::filesystem::directory_iterator{this->config.mock.images_dir},
        [this](const auto& dir_entry) {
            cv::Mat img = cv::imread(dir_entry.path().string());
            // if the image is read 
            if (img.data != NULL) {
                ImageData img_data{
                    img,
                    ImageTelemetry(38.31568, 76.55006, 75, 20, 0, 100, 5, 3)};
                this->mock_images.push_back(img_data);
            }
        }
    );
}

MockCamera::~MockCamera() {
    this->stopTakingPictures();
}

void MockCamera::connect() { return; }

bool MockCamera::isConnected() { return true; }

void MockCamera::startTakingPictures(const std::chrono::milliseconds& interval) {
    this->isTakingPictures = true;
    try {
        this->captureThread = std::thread(&MockCamera::captureEvery, this, interval);
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

void MockCamera::captureEvery(const std::chrono::milliseconds& interval) {
    loguru::set_thread_name("mock camera");
    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with mock camera. Using images from %s",
            this->config.mock.images_dir.c_str());
        ImageData newImage = this->takePicture();

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push_back(newImage);
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
} 

ImageData MockCamera::takePicture() {
    int random_idx = randomInt(0, this->mock_images.size()-1);
    return this->mock_images.at(random_idx);
}

