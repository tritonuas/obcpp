#include <deque>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/interface.hpp"
#include "camera/lucid.hpp"


using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    json config = 
    CameraConfiguration config(json::parse(R"(
{
    "TriggerSelector":"FrameStart",
    "TriggerMode":"On",
    "TriggerSource":"Software",
    "SensorShutterMode":"Rolling",
    "ExposureAuto":"Continuous",
    "AcquisitionFrameRateEnable":true,
    "TargetBrightness": 70,
    "GammaEnable":true,
    "Gamma": 0.5,
    "GainAuto":"Continuous",
    "ExposureAutoDamping":1,
    "ExposureAutoAlgorithm":"Median",
    "ExposureAutoUpperLimit": 500,
    "ExposureAutoLowerLimit": 360,
    "GainAutoUpperLimit": 10,
    "GainAutoLowerLimit": 1,
    "StreamAutoNegotiatePacketSize":true,  
    "StreamPacketResendEnable":true
}
)"));
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