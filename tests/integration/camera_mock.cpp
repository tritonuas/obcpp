#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "camera/mock.hpp"
#include "core/mission_state.hpp"

#include <memory>
#include <optional>

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    std::shared_ptr<MissionState> state = std::make_shared<MissionState>();
    CameraConfig config;
    config.type = "mock";
    config.mock.images_dir =  "/workspaces/obcpp/tests/integration/images/saliency/";
    MockCamera camera(config);

    camera.connect();

    camera.startTakingPictures(1s);
    std::this_thread::sleep_for(2s);
    std::optional<ImageData> image = camera.getLatestImage();
    if (image.has_value()) {
        cv::imwrite("mock_img.jpg", image.value().getData());
    }
    camera.stopTakingPictures();
}