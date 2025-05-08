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

#include "camera/interface.hpp"
#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/common.hpp"

#include "camera/rpi.hpp"

const uint32_t IMG_WIDTH = 2028;
const uint32_t IMG_HEIGHT = 1520;
const uint32_t BUFFER_SIZE = IMG_WIDTH * IMG_HEIGHT * 3 / 2;


RPICamera::RPICamera(CameraConfig config, asio::io_context* io_context_) : CameraInterface(config), client(io_context_, SERVER_IP, SERVER_PORT) {
// TODO: do we need this? not sure how we're configuring the camera if that's being done on Daniel's side

}

// TODO
void RPICamera::connect() {
    if (this->connected == true) {
        return;
    }
	
	while (!this->connected) {
        this->connected = client.connect();
	}

	// TODO: switch to LOG_F?
	std::cout << "Connected to: " << client.getIP() << " on port: " << client.getPort() << '\n';
	
    // tells the camera to start the camera thread
	client.send(START_REQUEST);
}

RPICamera::~RPICamera() {

}

std::optional<ImageData> RPICamera::takePicture(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) {
	
	// client sends a request to take a pictures
	client.send(PICTURE_REQUEST);

	// client receives a response TODO: might have to adjust the datatype
	std::vector<std::uint8_t> imgbuf = client.recv(BUFFER_SIZE);

	// TODO: have to parse the imgbuf possibly depending on how the response is structured?

	// give the image buffer to imgConvert
	std::optional<cv::Mat> mat = imgConvert(imgbuf);

	if (!mat.has_value()) {
		return {};
	}

    uint64_t timestamp = getUnixTime_s().count();

	return ImageData {
		.DATA = mat.value(),
		.TIMESTAMP = timestamp,
		.TELEMETRY = {}
	};
}

std::optional<cv::Mat> RPICamera::imgConvert(std::vector<std::uint8_t> buf) {
	// TODO: if the sizes don't match return nullopt
    if (buf.size() != BUFFER_SIZE) {
        return {};
    }

    // put raw bytes in cv::Mat
    cv::Mat yuv420_img(IMG_HEIGHT * 3 / 2, IMG_WIDTH, CV_8UC1, buf.data());

    cv::Mat bgr_img(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
    cv::cvtColor(yuv420_img, bgr_img, cv::COLOR_YUV2BGR_I420); // TODO: not sure if this is the right color space

    return bgr_img;
}

void RPICamera::startTakingPictures(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) {

}

void RPICamera::stopTakingPictures() {

}

void RPICamera::ping() {

}

void RPICamera::readImage() {

}

void RPICamera::readTelemetry() {

}

void RPICamera::startStreaming() {

}

bool RPICamera::isConnected() {
    return true;
}
