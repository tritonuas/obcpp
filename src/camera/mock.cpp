#include "camera/mock.hpp"

#include <chrono>
#include <thread>
#include <optional>
#include <deque>
#include <filesystem>
#include <httplib.h>

#include <loguru.hpp>
#include "nlohmann/json.hpp"

#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/rng.hpp"
#include "utilities/common.hpp"
#include "utilities/base64.hpp"


MockCamera::MockCamera(CameraConfig config) : CameraInterface(config)
{
    LOG_F(INFO, "Grabbing images from port " + config.mock.not_stolen_port);
    httplib::Client cli("localhost", config.mock.not_stolen_port);

    cli.set_read_timeout(10);

    uint32_t image_count = 10;

    for (int i = 0; i < image_count; i++)
    {
        auto res = cli.Get("/generate?format=json");

        if (!res || res->status != 200)
        {
            LOG_F(ERROR, "Failed to query server for images");
            continue;
        }

        nlohmann::json json = nlohmann::json::parse(res->body, nullptr, true, true);

        std::string decoded_img = base64_decode(json["image_base64"]);
        std::vector<uchar> data(decoded_img.begin(), decoded_img.end());

        cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);

        if (img.data == NULL)
        {
            LOG_F(ERROR, "Failed to decode image from server response");
            continue;
        } 
        
        ImageTelemetry telemetry = this->getTelemetryFromJsonResponse(res->body).value();

        ImageData img_data(
            img,
            0,
            telemetry);

        this->mock_images.push_back(img_data);
    }

    cli.stop();
}

MockCamera::~MockCamera()
{
    this->stopTakingPictures();
}

void MockCamera::connect() { return; }

bool MockCamera::isConnected() { return true; }

void MockCamera::startTakingPictures(const std::chrono::milliseconds &interval,
                                     std::shared_ptr<MavlinkClient> mavlinkClient)
{
    this->isTakingPictures = true;
    try
    {
        this->captureThread = std::thread(&MockCamera::captureEvery, this, interval, mavlinkClient);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void MockCamera::stopTakingPictures()
{
    if (!this->isTakingPictures)
    {
        return;
    }

    this->isTakingPictures = false;

    this->captureThread.join();
}

std::optional<ImageData> MockCamera::getLatestImage()
{
    ReadLock lock(this->imageQueueLock);
    ImageData lastImage = this->imageQueue.front();
    this->imageQueue.pop_front();
    return lastImage;
}

std::deque<ImageData> MockCamera::getAllImages()
{
    ReadLock lock(this->imageQueueLock);
    std::deque<ImageData> outputQueue = this->imageQueue;
    this->imageQueue = std::deque<ImageData>();
    return outputQueue;
}

void MockCamera::captureEvery(const std::chrono::milliseconds &interval,
                              std::shared_ptr<MavlinkClient> mavlinkClient)
{
    loguru::set_thread_name("mock camera");
    while (this->isTakingPictures)
    {
        LOG_F(INFO, "Taking picture with mock camera. Using images from port %d",
              this->config.mock.not_stolen_port);
        auto imageData = this->takePicture(interval, mavlinkClient);

        if (!imageData.has_value())
        {
            LOG_F(WARNING, "Failed to take picture with mock camera");
            continue;
        }

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push_back(imageData.value());
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }
}

std::optional<ImageData> MockCamera::takePicture(const std::chrono::milliseconds &timeout,
                                                 std::shared_ptr<MavlinkClient> mavlinkClient)
{
    int random_idx = randomInt(0, this->mock_images.size() - 1);

    ImageData img_data = this->mock_images.at(random_idx);
    uint64_t timestamp = getUnixTime_s().count();

    // if we can't find corresonding telemtry json, just query mavlink
    if (!img_data.TELEMETRY.has_value())
    {
        LOG_F(ERROR, "no image json value");
        img_data.TELEMETRY = queryMavlinkImageTelemetry(mavlinkClient);
    }

    ImageData imageData{
        .DATA = img_data.DATA,
        .TIMESTAMP = timestamp,
        .TELEMETRY = img_data.TELEMETRY,
    };

    return imageData;
}

void MockCamera::startStreaming() {}

std::optional<ImageTelemetry> MockCamera::getTelemetryFromJsonResponse(std::string server_response)
{
    nlohmann::json json = nlohmann::json::parse(server_response, nullptr, true, true);

    return ImageTelemetry{
        .latitude_deg = json["background"]["lat"],
        .longitude_deg = json["background"]["lon"],
        .altitude_agl_m = json["altitude"],
        .airspeed_m_s = 0.0,
        .heading_deg = json["background"]["heading_deg"],
        .yaw_deg = 0.0,
        .pitch_deg = 0.0,
        .roll_deg = 0.0,
    };
}
