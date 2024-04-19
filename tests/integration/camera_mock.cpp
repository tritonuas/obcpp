#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "camera/mock.hpp"

#include <optional>

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    CameraConfiguration config({
       {"SampleConfigKey", 100},
       {"ExposureTime", 1000},
    });
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