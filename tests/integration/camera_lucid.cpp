#include <deque>

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

    LucidCamera camera(config.camera_config);

    camera.connect();
    LOG_F(INFO, "Connected to LUCID camera!");

    camera.startTakingPictures(1s, mav);

    // need to sleep to let camera background thread to run
    std::this_thread::sleep_for(10s);
    camera.stopTakingPictures();

    std::deque<ImageData> images = camera.getAllImages();
    for (const ImageData& image : images) {
        std::filesystem::path output_file =  
            output_dir / 
            (std::string("lucid_") + std::to_string(getUnixTime_ms().count()) + std::string(".jpg"));
        LOG_F(INFO, "Saving LUCID image to %s", output_file.string().c_str());
        LOG_F(INFO, "lat: %f, lon: %f, alt: %f, hdg: %f, ptc: %f, rol: %f", 
			image.TELEMETRY.value().latitude_deg, 
			image.TELEMETRY.value().longitude_deg, 
			image.TELEMETRY.value().altitude_agl_m, 
			image.TELEMETRY.value().heading_deg, 
			image.TELEMETRY.value().pitch_deg, 
			image.TELEMETRY.value().roll_deg);
        cv::imwrite(output_file, image.DATA);
    }
}
