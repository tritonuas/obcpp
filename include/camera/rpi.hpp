#ifndef INCLUDE_CAMERA_RPI_HPP_
#define INCLUDE_CAMERA_RPI_HPP_


#include <string>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <deque>

#include <nlohmann/json.hpp>

#include "camera/interface.hpp"
#include "network/mavlink.hpp"

using json = nlohmann::json;

using namespace std::chrono_literals; // NOLINT

#include "interface.hpp"
#include "network/client.hpp"

namespace asio = boost::asio;

const std::string SERVER_IP = "192.168.68.1";
const int SERVER_PORT = 25565;
// const std::string SERVER_IP = "127.0.0.1";
// const int SERVER_PORT = 5000;
const std::uint8_t START_REQUEST = 's';
const std::uint8_t PICTURE_REQUEST = 'p';
const std::uint8_t END_REQUEST = 'e';
const std::uint8_t LOCK_REQUEST = 'l';

class RPICamera : public CameraInterface {
	private:
		Client client;
        asio::io_context io_context_;

        std::atomic_bool connected;

		std::deque<ImageData> imageQueue; // TODO: unsure if we actually need this if we're just gonna directly save images to disk

		// lock for obc client?
		// lock for imageQueue?	

        /**
         * Converts the image taken from the camera to a suitable format for the CV pipeline
         */
		std::optional<cv::Mat> imgConvert(std::vector<std::uint8_t> imgbuf);

        /**
         * Reads in the image data when taking a picture
         */
        void readImage();

        /**
         * Reads in the telemetry data when taking a picture
         */
        void readTelemetry();

	public:
    
		explicit RPICamera(CameraConfig config, asio::io_context* io_context_);

		// TODO: destructor?
		~RPICamera();
	
		void connect() override;
		bool isConnected() override;

		std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) override;

        void startTakingPictures(const std::chrono::milliseconds& interval,
            std::shared_ptr<MavlinkClient> mavlinkClient) override;
        void stopTakingPictures() override;

        void startStreaming() override;
        
        // TODO: unsure how to implement
        void ping();
};

#endif
