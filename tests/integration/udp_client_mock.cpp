#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <loguru.hpp>
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
        std::filesystem::path base_dir = "/workspaces/obcpp/images";
        std::filesystem::path filepath = base_dir / std::to_string(img.value().TIMESTAMP);
        img.value().saveToFile(filepath);
    }

    return 0;
}
