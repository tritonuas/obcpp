#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "camera/lucid.hpp"


using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    LucidCamera camera(nullptr);

    camera.connect();

    camera.startTakingPictures(1s);
    std::this_thread::sleep_for(2s);
    std::optional<ImageData> image = camera.getLatestImage();
    if (image.has_value()) {
        cv::imwrite("lucid_img.jpg", image.value().getData());
    }
    camera.stopTakingPictures();
}