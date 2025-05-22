#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <loguru.hpp>
#include <opencv2/opencv.hpp>
#include "network/udp_client.hpp"
#include "camera/rpi.hpp"
#include "utilities/obc_config.hpp"
#include "core/mission_state.hpp"
#include "utilities/common.hpp"

int main(int argc, char* argv[]) {

    CameraConfig config;

    asio::io_context io_context_;

    RPICamera camera(config, &io_context_);

    camera.connect();

    std::optional<ImageData> img = camera.takePicture(std::chrono::milliseconds(1000), nullptr);

    if (img.has_value()) {
        LOG_F(INFO, "Image has value");
        // cv::imshow("img", img.value().DATA);
        // cv::waitKey(0);
        std::filesystem::path base_dir = "/workspaces/obcpp/images";
        std::filesystem::path filepath = base_dir / std::to_string(img.value().TIMESTAMP) / ".jpg";
        img.value().saveToFile(std::filesystem::current_path()); // TODO: able to save images but idk what the correct file path should be
        std::cout << "Saving image to: " << filepath << std::endl;
    }

    return 0;
}
