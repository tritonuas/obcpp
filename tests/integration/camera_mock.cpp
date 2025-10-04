#include <memory>
#include <optional>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "camera/mock.hpp"
#include "utilities/datatypes.hpp"

using namespace std::chrono_literals;

int main (int argc, char *argv[]) {
    loguru::init(argc, argv);
    
    LOG_F(INFO, "Starting MockCamera test");

    CameraConfig config;
    config.type = "mock";
    config.save_dir = "/workspaces/obcpp/images/mock";
    config.save_images_to_file = true;
    config.mock.not_stolen_port = 6060;
    config.mock.lat = 38.31587003355332;
    config.mock.lon = -76.55251800009955;
    config.mock.alt_ft = 150;
    config.mock.heading = 270;

    std::filesystem::create_directories(config.save_dir);

    try {
        MockCamera camera(config);
        
        LOG_F(INFO, "Mock Camera created successfully");

        camera.connect();
        
        if (camera.isConnected()) {
            LOG_F(INFO, "Camera connected");
        } else {
            LOG_F(ERROR, "Camera connection failed");
            return 1;
        }

        camera.startStreaming();
        LOG_F(INFO, "Started streaming");

        LOG_F(INFO, "Taking 3 test pictures...");
        for (int i = 0; i < 3; i++) {
            auto imageData = camera.takePicture(5s, nullptr);
            if (imageData.has_value()) {
                LOG_F(INFO, "Picture %d: %dx%d pixels", i+1, 
                      imageData->DATA.cols, imageData->DATA.rows);


                std::string filename = "test_image_" + std::to_string(i) + ".jpg";
                std::filesystem::path filepath = std::filesystem::path(config.save_dir) / filename;
                cv::imwrite(filepath.string(), imageData->DATA);
                LOG_F(INFO, "Saved to %s", filepath.string().c_str());
            } else {
                LOG_F(WARNING, "Failed to take picture %d", i+1);
            }
            
            std::this_thread::sleep_for(1s);
        }

        LOG_F(INFO, "Mock Camera test completed successfully");
        
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception: %s", e.what());
        return 1;
    }

    return 0;
}