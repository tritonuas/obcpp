#include "camera/mock.hpp"

#include <chrono>
#include <thread>
#include <optional>
#include <deque>
#include <filesystem>

#include <loguru.hpp>
#include "nlohmann/json.hpp"

#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/rng.hpp"
#include "utilities/common.hpp"
#include <loguru.hpp>


MockCamera::MockCamera(CameraConfig config) : CameraInterface(config) {
    std::ranges::for_each(
        std::filesystem::directory_iterator{this->config.mock.images_dir},
        [this](const auto& dir_entry) {
            cv::Mat img = cv::imread(dir_entry.path().string());
            // if the image is read
            if (img.data != NULL) {
                std::optional<ImageTelemetry> telemetry =
                    this->getTelemetryFromJsonFile(dir_entry.path());

                ImageData img_data(
                    img,
                    0,
                    telemetry);
                this->mock_images.push_back(img_data);
            } else {
                LOG_F(ERROR, "IMG DIRECTORY IS EMPTY | RIP CAT");
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

        if (!imageData.has_value()) {
            LOG_F(WARNING, "Failed to take picture with mock camera");
            continue;
        }

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push_back(imageData.value());
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
}

std::optional<ImageData> MockCamera::takePicture(const std::chrono::milliseconds& timeout,
        std::shared_ptr<MavlinkClient> mavlinkClient) {
    LOG_F(ERROR, "try take picture");
    int random_idx = randomInt(0, this->mock_images.size()-1);

    ImageData img_data = this->mock_images.at(random_idx);
    uint64_t timestamp = getUnixTime_s().count();

    // if we can't find corresonding telemtry json, just query mavlink
    if (!img_data.TELEMETRY.has_value()) {
        img_data.TELEMETRY = queryMavlinkImageTelemetry(mavlinkClient);
    }

    ImageData imageData {
        .DATA = img_data.DATA,
        .TIMESTAMP = timestamp,
        .TELEMETRY = img_data.TELEMETRY,
    };



    LOG_F(ERROR, "take picture yay");
    return imageData;
}

void MockCamera::startStreaming() {}

std::optional<ImageTelemetry> MockCamera::getTelemetryFromJsonFile(std::filesystem::path img_path) {
    img_path.replace_extension("json");
    std::ifstream telemetry_stream(img_path);
    if (!telemetry_stream.is_open()) {
        // no corresponding telemetry json found
        return {};
    }
    nlohmann::json json = nlohmann::json::parse(telemetry_stream, nullptr, true, true);
    return ImageTelemetry {
        .latitude_deg = json["latitude_deg"],
        .longitude_deg = json["longitude_deg"],
        .altitude_agl_m = json["altitude_agl_m"],
        .airspeed_m_s = json["airspeed_m_s"],
        .heading_deg = json["heading_deg"],
        .yaw_deg = json["yaw_deg"],
        .pitch_deg = json["pitch_deg"],
        .roll_deg = json["roll_deg"],
    };
}
