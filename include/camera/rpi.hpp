#ifndef INCLUDE_CAMERA_RPI_HPP_
#define INCLUDE_CAMERA_RPI_HPP_

#include <string>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <deque>
#include <vector>

#include <nlohmann/json.hpp>
#include "camera/interface.hpp"
#include "network/mavlink.hpp"
#include "network/udp_client.hpp"

using json = nlohmann::json;
using namespace std::chrono_literals; // NOLINT

namespace asio = boost::asio;

const std::uint8_t START_REQUEST = 's';
const std::uint8_t PICTURE_REQUEST = 'I';
const std::uint8_t END_REQUEST = 'e';
const std::uint8_t LOCK_REQUEST = 'l';

class RPICamera : public CameraInterface {
    private:
        UDPClient client;
        asio::io_context io_context_;
        std::atomic_bool connected;

        /**
         * Converts the 3-plane raw data to BGR cv::Mat, handling stride/padding
         */
        std::optional<cv::Mat> imgConvert(const std::vector<std::vector<uint8_t>>& planes);

        /**
         * Reads the 3 planes (Y, U, V) from the camera
         */
        std::vector<std::vector<uint8_t>> readImage();

    public:
        explicit RPICamera(CameraConfig config, asio::io_context* io_context_);
        ~RPICamera();
    
        void connect() override;
        bool isConnected() override;

        std::optional<ImageData> getLatestImage() override {return std::nullopt;}
        std::deque<ImageData> getAllImages() override {return std::deque<ImageData>();}
        
        std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) override;

        void startTakingPictures(const std::chrono::milliseconds& interval, std::shared_ptr<MavlinkClient> mavlinkClient) override;
        void stopTakingPictures() override;
        void startStreaming() override;
        void ping();
};

#endif