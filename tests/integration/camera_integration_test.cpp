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

    std::cout << "[Test] Initializing RPICamera..." << std::endl;
    RPICamera camera(config, &io_context_);

    std::cout << "[Test] Connecting..." << std::endl;
    camera.connect();

    if (!camera.isConnected()) {
        std::cerr << "[Test] Failed to connect/bind socket" << std::endl;
        return -1;
    }

    std::cout << "[Test] Requesting Picture..." << std::endl;
    std::optional<ImageData> img = camera.takePicture(std::chrono::milliseconds(3000), nullptr);

    if (img.has_value()) {
        std::cout << "[Test] Image Received! Size: " << img.value().DATA.total() << " bytes." << std::endl;
        
        fs::path base_dir = "images";
        if (!fs::exists(base_dir)) {
            fs::create_directory(base_dir);
        }

        fs::path filepath = base_dir / (std::to_string(img.value().TIMESTAMP) + ".png");
        
        if (img.value().saveToFile(base_dir.string())) {
             std::cout << "[Test] Saved successfully to " << base_dir << std::endl;
        } else {
             std::cerr << "[Test] Failed to save image file." << std::endl;
        }
    } else {
        std::cerr << "[Test] FAILED: No image received (Timeout or Error)." << std::endl;
        return 1;
    }

    return 0;
}