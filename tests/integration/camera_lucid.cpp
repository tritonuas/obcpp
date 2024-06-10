#include <deque>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/interface.hpp"
#include "camera/lucid.hpp"
#include "core/mission_state.hpp"
#include "network/mavlink.hpp"
#include "utilities/common.hpp"

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    if (argc < 2) {
        LOG_F(ERROR, "Usage: ./bin/camera_lucid [path_to_config] [optional: output_dir]");
        exit(1);
    }
    std::filesystem::path output_dir = std::filesystem::current_path();
    if (argc >= 3) {
        output_dir = argv[2];
    }
    OBCConfig config(argc, argv);

    auto mav = std::make_shared<MavlinkClient>("serial:///dev/ttyACM0");

    LucidCamera camera(config.camera);

    camera.connect();
    LOG_F(INFO, "Connected to LUCID camera!");

    camera.startTakingPictures(1s, mav);

    // need to sleep to let camera background thread to run
    std::this_thread::sleep_for(10s);
    camera.stopTakingPictures();

    std::deque<ImageData> images = camera.getAllImages();
    for (const ImageData& image : images) {
        std::filesystem::path filepath = config.camera.save_dir / std::to_string(image.TIMESTAMP);
        saveImageToFile(image.DATA, filepath);
        if (image.TELEMETRY.has_value()) {
            saveImageTelemetryToFile(image.TELEMETRY.value(), filepath);
        }
        LOG_F(INFO, "Saving LUCID image to %s", filepath.string().c_str());
    }
}
