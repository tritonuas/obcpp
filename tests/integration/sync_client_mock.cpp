#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "network/client.hpp"
#include "camera/rpi.hpp"
#include "utilities/obc_config.hpp"

int main(int argc, char* argv[]) {

    OBCConfig config(argc, argv);

    asio::io_context io_context_;

    RPICamera camera(config.camera, &io_context_);
    // RPICamera camera(&io_context_);

    std::deque<ImageData> imageQueue;

    camera.connect();

    std::optional<ImageData> img = camera.takePicture(std::chrono::milliseconds(1000));

    if (img.has_value()) {
        std::filesystem::path filepath = std::to_string(img.value().TIMESTAMP);
        img.value().saveToFile(filepath);
    }

    return 0;
}
