#include <deque>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/interface.hpp"
#include "camera/lucid.hpp"


using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    CameraConfiguration config({
       {"SampleConfigKey", 100},
       {"ExposureTime", 1000},
    });
    LucidCamera camera(config);

    LOG_F(INFO, "Trying to connect to LUCID camera\n");
    camera.connect();
    LOG_F(INFO, "Connected to LUCID camera!\n");

    camera.startTakingPictures(1s);

    std::this_thread::sleep_for(10s);

    std::deque<ImageData> images = camera.getAllImages();
    int imageNum = 0;
    for (const ImageData& image : images) {
        cv::imwrite("lucid_img" + std::to_string(imageNum) + ".jpg", image.getData());
        imageNum++;
    }

    camera.stopTakingPictures();
}