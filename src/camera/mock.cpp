#include "camera/mock.hpp"

#include <httplib.h>

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
#include "utilities/base64.hpp"


MockCamera::MockCamera(CameraConfig config)
    : CameraInterface(config), cli("localhost", config.mock.not_stolen_port) {}

MockCamera::~MockCamera() {
    cli.stop();
    this->stopTakingPictures();
}

void MockCamera::connect() { return; }

bool MockCamera::isConnected() {
    cli.set_read_timeout(2);
    httplib::Result res = cli.Get("/");
    return res && (res->status == 200 || res->status == 404);
}

void MockCamera::startTakingPictures(const std::chrono::milliseconds& interval,
                                     std::shared_ptr<MavlinkClient> mavlinkClient) {
    this->isTakingPictures = true;
    try {
        this->captureThread = std::thread(&MockCamera::captureEvery, this, interval, mavlinkClient);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MockCamera::stopTakingPictures() {
    if (!this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = false;

    nlohmann::json json;
    json["session_id"] = this->session_id;

    httplib::Result res = cli.Post("/stream/stop", json.dump(), "application/json");

    if (!res || res->status != 200) {
        LOG_F(WARNING, "Failed to stop streaming session");
    } else {
        LOG_F(INFO, "Successfully stopped streaming session");
    }

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
    cli.set_read_timeout(10);
    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with mock camera. Using images from port %d",
              this->config.mock.not_stolen_port);
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
    std::optional<ImageTelemetry> telemetryOpt = queryMavlinkImageTelemetry(mavlinkClient);

    if (!telemetryOpt.has_value()) {
        LOG_F(ERROR, "Could not grab telemetry data from mavlink");
        return std::nullopt;
    }

    ImageTelemetry telemetry = telemetryOpt.value();

    httplib::Result res = cli.Get("/stream/frame?session_id=" + this->session_id +
                                  "&lat=" + std::to_string(telemetry.latitude_deg) +
                                  "&lon=" + std::to_string(telemetry.longitude_deg) +
                                  "&alt_ft=" + std::to_string(static_cast<int>((telemetry.altitude_agl_m * 3.281))) + //NOLINT
                                  "&heading=" + std::to_string(telemetry.heading_deg) +
                                  "&format=png");

    if (!res || res->status != 200) {
        LOG_F(ERROR, "Failed to query server for images");
        return std::nullopt;
    }

    std::vector<uchar> data(res->body.begin(), res->body.end());
    cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);

    if (img.data == NULL) {
        LOG_F(ERROR, "Failed to decode image from server response");
        return std::nullopt;
    }

    ImageData img_data(
        img,
        0,
        telemetry);

    return img_data;
}

void MockCamera::startStreaming() {
    nlohmann:json stream_body;
    stream_body["runway"] = this->config.mock.runway;
    stream_body["num_targets"] = this->config.mock.num_targets;

    httplib::Result res = cli.Post("/stream/start", stream_body.dump(), "application/json");

    if (!res || res->status != 200) {
        LOG_F(ERROR, "Failed to grab session id from not-stolen");
        return;
    }

    nlohmann::json json = nlohmann::json::parse(res->body, nullptr, true, true);

    this->session_id = json["session_id"];

    LOG_F(INFO, "Started streaming with session id %s", this->session_id.c_str());
}
