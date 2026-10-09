#include <iostream>
#include <vector>
#include <filesystem>
#include <boost/asio.hpp>
#include <loguru.hpp>
#include "camera/rpi.hpp"
#include "utilities/obc_config.hpp"
#include "core/mission_state.hpp"
#include "utilities/common.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    loguru::init(argc, argv);

    CameraConfig config;
    asio::io_context io_context_;
    const int NUM_IMAGES = 5;
    const std::chrono::milliseconds IMAGE_INTERVAL = std::chrono::milliseconds(250);

    LOG_F(INFO, "[Test] Initializing RPICamera...");
    RPICamera camera(config, &io_context_);

    LOG_F(INFO, "[Test] Connecting...");
    camera.connect();

    if (!camera.isConnected()) {
        LOG_F(ERROR, "[Test] Failed to connect/bind socket");
        return -1;
    }

    for (int i = 0; i < NUM_IMAGES; i++) {
        LOG_F(INFO, "[Test] Requesting Picture %d/%d", i + 1, NUM_IMAGES);
        std::this_thread::sleep_for(IMAGE_INTERVAL);
        std::optional<ImageData> img = camera.takePicture(std::chrono::milliseconds(3000), nullptr);

        if (img.has_value()) {
            LOG_F(INFO, "[Test] Image Received! Size: %ld bytes.", img.value().DATA.total());
            
            fs::path base_dir = "images";
            if (!fs::exists(base_dir)) {
                fs::create_directory(base_dir);
            }

            fs::path filepath = base_dir / (std::to_string(img.value().TIMESTAMP) + ".png");
            
            if (img.value().saveToFile(base_dir.string())) {
                LOG_F(INFO, "[Test] Saved successfully to %s", base_dir.c_str());
            } else {
                LOG_F(ERROR, "[Test] Failed to save image file.");
            }
        } else {
            LOG_F(ERROR, "[Test] FAILED: No image received (Timeout or Error).");
            return 1;
        }
    }

    return 0;
}
