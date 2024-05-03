#include <deque>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/interface.hpp"
#include "camera/lucid.hpp"
#include "core/mission_state.hpp"

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    CameraConfig camera_config;
    camera_config.type = "lucid";

    camera_config.lucid.sensor_shutter_mode = "Rolling";
    camera_config.lucid.acquisition_frame_rate_enable = true;
    camera_config.lucid.stream_auto_negotiate_packet_size = true;
    camera_config.lucid.stream_packet_resend_enable = true;
    camera_config.lucid.target_brightness = 70;
    camera_config.lucid.gamma_enable = true;
    camera_config.lucid.gamma = 0.5;
    camera_config.lucid.gain_auto = "Continuous";
    camera_config.lucid.gain_auto_upper_limit = 10;
    camera_config.lucid.gain_auto_lower_limit = 1;
    camera_config.lucid.exposure_auto = "Continuous";
    camera_config.lucid.exposure_auto_damping = 1;
    camera_config.lucid.exposure_auto_algorithm = "Median";
    camera_config.lucid.exposure_auto_upper_limit = 500;
    camera_config.lucid.exposure_auto_lower_limit = 360;

    LucidCamera camera(camera_config);

    LOG_F(INFO, "Trying to connect to LUCID camera\n");
    camera.connect();
    LOG_F(INFO, "Connected to LUCID camera!\n");

    camera.startTakingPictures(0s);

    std::this_thread::sleep_for(30s);

    camera.stopTakingPictures();

    std::deque<ImageData> images = camera.getAllImages();
    int imageNum = 0;
    for (const ImageData& image : images) {
        cv::imwrite("lucid_img" + std::to_string(imageNum) + ".jpg", image.getData());
        imageNum++;
    }
}