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

// const uint32_t IMG_WIDTH = 2028;
// const uint32_t IMG_HEIGHT = 1520;
// const uint32_t IMG_BUFFER = IMG_WIDTH * IMG_HEIGHT * 3 / 2; // normal image size
const uint32_t IMG_SIZE = 4668440;


RPICamera::RPICamera(CameraConfig config, asio::io_context* io_context_) : CameraInterface(config), client(io_context_, SERVER_IP, SERVER_PORT) {
    this->connected = false;
}

void RPICamera::connect() {
    if (this->connected == true) {
        return;
    }
	
	while (!this->connected) {
        this->connected = client.connect();
	}
	
    // tells the camera to start the camera thread
	client.send(START_REQUEST);
}

RPICamera::~RPICamera() {
    // TODO: probably have to shutdown/free the socket and free the iocontext if that's a thing
}

std::optional<ImageData> RPICamera::takePicture(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) {
	
	// client sends a request to take a pictures
	client.send(PICTURE_REQUEST);

    // TODO: get the imgbuf
    std::vector<std::uint8_t> imgbuf = readImage();

	// give the image buffer to imgConvert
	std::optional<cv::Mat> mat = imgConvert(imgbuf);

	if (!mat.has_value()) {
		return {};
	}

    uint64_t timestamp = getUnixTime_s().count();

	return ImageData {
		.DATA = mat.value(),
		.TIMESTAMP = timestamp,
		.TELEMETRY = {} // TODO: nullopt for now
	};
}

std::optional<cv::Mat> RPICamera::imgConvert(std::vector<std::uint8_t> buf) {

    // TODO: how to check the expected buffer size idk, counter?
    // if (buf.size() != BUFFER_SIZE) {
    //     return {};
    // }

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

std::vector<std::uint8_t> RPICamera::readImage() {
    // 3 separate reads for 3 separate files

    std::vector<std::uint8_t> imgbuf;

    for (int i = 0; i < 3; i++) {
        std::vector<std::uint8_t> packet = readPacket();

        // have to concatenate the packets since there are 3 "image files"
        imgbuf.reserve(imgbuf.size() + packet.size());
        imgbuf.insert(imgbuf.end(), packet.begin(), packet.end());
    }

    return imgbuf;
}

std::optional<ImageTelemetry> RPICamera::readTelemetry() {

    ImageTelemetry telemetry;
    // asio::read()

    return telemetry;
}

std::vector<std::uint8_t> RPICamera::readPacket() {

    std::vector<std::uint8_t> packet;

    Header header = client.recvHeader();

    // TODO: ntohl or ntohs?
    header.magic = ntohl(header.magic);
    header.mem_size = ntohl(header.mem_size);
    header.total_chunks = ntohl(header.total_chunks);

    // check the magic number, sort of like a checksum ig
    if (header.magic != EXPECTED_MAGIC) {
        // TODO: how do we even handle this, after we read the corrupted header we don't even know how many bytes to throw away to read the next header
    }

    packet = client.recvBody(header.mem_size * header.total_chunks);

    // TODO: idek if we have to do ntoh for the buffer
    
    return packet;
}

void RPICamera::startStreaming() {

}

bool RPICamera::isConnected() {
    return true;
}
