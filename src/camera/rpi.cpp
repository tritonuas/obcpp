#include <chrono>
#include <thread>
#include <optional>
#include <string>
#include <vector>
#include <deque>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/rpi.hpp"
#include "network/rpi_connection.hpp" 

// Setup Logging
// ...

RPICamera::RPICamera(CameraConfig config, asio::io_context* io_context_) 
    : CameraInterface(config), client(io_context_, SERVER_IP, SERVER_PORT) {
    this->connected = false;
}

void RPICamera::connect() {
    if (this->connected) return;
    
    // Keep trying to connect/bind logic
    // For UDP, "connect" just means opening the socket which is fast
    if (client.connect()) {
        this->connected = true;
        // Optionally send START_REQUEST if needed, but 'I' usually works standalone (i think)
        // client.send(START_REQUEST);
    }
}

RPICamera::~RPICamera() {}

std::optional<ImageData> RPICamera::takePicture(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) {
    
    // 1. Send Request
    if (!client.send(PICTURE_REQUEST)) {
        LOG_F(ERROR, "Failed to send picture request");
        return {};
    }

    // 2. Read 3 Planes
    std::vector<std::vector<uint8_t>> planes = readImage();

    if (planes.size() != 3) {
        LOG_F(ERROR, "Failed to read all 3 planes");
        return {};
    }

    // 3. Convert to cv::Mat
    std::optional<cv::Mat> mat = imgConvert(planes);

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

std::vector<std::vector<uint8_t>> RPICamera::readImage() {
    std::vector<std::vector<uint8_t>> planes;

    // We expect exactly 3 planes: Y, U, V
    for (int i = 0; i < 3; i++) {
        Header header = client.recvHeader();
        
        // Convert endianness
        header.magic = ntohl(header.magic);
        header.mem_size = ntohl(header.mem_size);
        header.total_chunks = ntohl(header.total_chunks);

        if (header.magic != EXPECTED_MAGIC) {
            LOG_F(ERROR, "Invalid Magic on plane %d: %x", i, header.magic);
            return {};
        }

        std::vector<uint8_t> planeData = client.recvBody(header.mem_size, header.total_chunks);
        if (planeData.empty()) {
            LOG_F(ERROR, "Failed to receive body for plane %d", i);
            return {};
        }
        
        planes.push_back(std::move(planeData));
    }

    return planes;
}

std::optional<cv::Mat> RPICamera::imgConvert(const std::vector<std::vector<uint8_t>>& planes) {
    // We rely on constants from rpi_connection.hpp:
    // IMG_WIDTH, IMG_HEIGHT, STRIDE_Y, STRIDE_UV

    if (planes.size() != 3) return {};

    const std::vector<uint8_t>& p0 = planes[0]; // Y
    const std::vector<uint8_t>& p1 = planes[1]; // U
    const std::vector<uint8_t>& p2 = planes[2]; // V

    // Create wrappers around the raw data with specific stride (Step)
    // Note: cv::Mat constructor with data pointer does not copy data, so we must be careful with lifetime
    // But we copy immediately below.
    cv::Mat y_src(IMG_HEIGHT, IMG_WIDTH, CV_8UC1, (void*)p0.data(), STRIDE_Y);
    cv::Mat u_src(IMG_HEIGHT/2, IMG_WIDTH/2, CV_8UC1, (void*)p1.data(), STRIDE_UV);
    cv::Mat v_src(IMG_HEIGHT/2, IMG_WIDTH/2, CV_8UC1, (void*)p2.data(), STRIDE_UV);

    // Allocate continuous buffer for standard I420
    cv::Mat yuv_continuous(IMG_HEIGHT + IMG_HEIGHT/2, IMG_WIDTH, CV_8UC1);

    // Copy Y (Removes padding)
    y_src.copyTo(yuv_continuous(cv::Rect(0, 0, IMG_WIDTH, IMG_HEIGHT)));

    // Copy U and V (Removes padding)
    // We need to flatten them carefully into the buffer after Y
    size_t y_size = IMG_WIDTH * IMG_HEIGHT;
    size_t uv_size = (IMG_WIDTH / 2) * (IMG_HEIGHT / 2);

    uint8_t* u_dst = yuv_continuous.data + y_size;
    uint8_t* v_dst = u_dst + uv_size;

    // Copy U
    if (u_src.isContinuous()) {
        memcpy(u_dst, u_src.data, uv_size);
    } else {
        for (int row = 0; row < u_src.rows; ++row) {
             memcpy(u_dst + row * (IMG_WIDTH/2), u_src.ptr(row), IMG_WIDTH/2);
        }
    }

    // Copy V
    if (v_src.isContinuous()) {
        memcpy(v_dst, v_src.data, uv_size);
    } else {
        for (int row = 0; row < v_src.rows; ++row) {
             memcpy(v_dst + row * (IMG_WIDTH/2), v_src.ptr(row), IMG_WIDTH/2);
        }
    }

    // Convert I420 to BGR
    cv::Mat bgr_img;
    cv::cvtColor(yuv_continuous, bgr_img, cv::COLOR_YUV2BGR_I420);

    return bgr_img;
}

// Unused methods stubbed out
void RPICamera::startTakingPictures(const std::chrono::milliseconds& timeout, std::shared_ptr<MavlinkClient> mavlinkClient) {}
void RPICamera::stopTakingPictures() {}
void RPICamera::startStreaming() {}
void RPICamera::ping() {}
bool RPICamera::isConnected() { return connected; }