#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "camera/mock.hpp"
#include "core/mission_state.hpp"

#include <memory>
#include <optional>

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    if (argc < 2) {
        LOG_F(ERROR, "Usage: ./bin/camera_mock [path_to_config]");
        exit(1);
    }
    OBCConfig config(argc, argv);
    MockCamera camera(config.camera_config);

    camera.connect();

    // start taking pictures every second
    camera.startTakingPictures(1s);
    // need to sleep to let camera background thread to run
    std::this_thread::sleep_for(2s);
    camera.stopTakingPictures();

    std::optional<ImageData> image = camera.getLatestImage();
    if (image.has_value()) {
        cv::imwrite("mock_img.jpg", image.value().getData());
    }
}