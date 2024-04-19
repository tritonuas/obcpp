#include "camera/mock.hpp"

#include <chrono>
#include <thread>
#include <optional>

#include <loguru.hpp>

#include "utilities/locks.hpp"


MockCamera::MockCamera(CameraConfiguration config) : CameraInterface(config) {}

MockCamera::~MockCamera() {
    this->stopTakingPictures();
}

void MockCamera::connect() { return; }

bool MockCamera::isConnected() { return true; }

void MockCamera::startTakingPictures(std::chrono::seconds interval) {
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
    this->imageQueue.pop();
    return lastImage;
}

std::queue<ImageData> MockCamera::getAllImages() {
    ReadLock lock(this->imageQueueLock);
    std::queue<ImageData> outputQueue = this->imageQueue; 
    this->imageQueue = std::queue<ImageData>();
    return outputQueue;
}

void MockCamera::captureEvery(std::chrono::seconds interval) {
    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with mock camera\n");
        ImageData newImage = this->takePicture();

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push(newImage);
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
} 

ImageData MockCamera::takePicture() {
    return ImageData("mock_image.jpg", "/real/path/mock_image.jpg",
                     cv::Mat(cv::Size(4000, 3000), CV_8UC3, cv::Scalar(255)),
                     ImageTelemetry(38.31568, 76.55006, 75, 20, 100, 5, 3));
}

