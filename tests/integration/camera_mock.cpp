#include <memory>
#include <optional>
#include <filesystem>
#include <string>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "camera/interface.hpp"
#include "camera/mock.hpp"
#include "utilities/common.hpp"

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    if (argc < 2) {
        LOG_F(ERROR, "Usage: ./bin/camera_mock [path_to_config] [optional: output_dir]");
        exit(1);
    }
    std::filesystem::path output_dir = std::filesystem::current_path();
    if (argc >= 3) {
        output_dir = argv[2];
    }
    OBCConfig config(argc, argv);

    MockCamera camera(config.camera);

    camera.connect();

    // start taking pictures every second
    camera.startTakingPictures(1s, nullptr);
    // need to sleep to let camera background thread to run
    std::this_thread::sleep_for(5s);
    camera.stopTakingPictures();

    std::deque<ImageData> images = camera.getAllImages();
    for (const ImageData& image : images) {
        std::filesystem::path save_dir = config.camera.save_dir;
        std::filesystem::path img_filepath = save_dir / (std::to_string(image.TIMESTAMP) + std::string(".jpg"));
        std::filesystem::path json_filepath = save_dir / (std::to_string(image.TIMESTAMP) + std::string(".json"));
        saveImageToFile(image.DATA, img_filepath);
        if (image.TELEMETRY.has_value()) {
            saveImageTelemetryToFile(image.TELEMETRY.value(), json_filepath);
        } 
        LOG_F(INFO, "Saving mock image to %s", img_filepath.string().c_str());
    }
}