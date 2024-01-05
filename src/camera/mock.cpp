#include "camera/mock.hpp"

MockCamera::MockCamera(CameraConfiguration config) : CameraInterface(config) {

}

void MockCamera::connect() {
    return;
};

bool MockCamera::verifyConnection() {
    return true;
}


void MockCamera::takePicture() {
    ImageData newImg("mock_image.jpg", "/real/path/mock_image.jpg",
        cv::Mat(cv::Size(4000, 3000), CV_8UC3, cv::Scalar(255)),
        ImageTelemetry(38.31568, 76.55006, 75, 20, 100, 5, 3));

    lastPicture = std::make_unique<ImageData>(newImg);
}

ImageData MockCamera::getLastPicture() {
    return *lastPicture;
}

bool MockCamera::takePictureForSeconds(int sec) {
    // TODO:
    return true;
};

void MockCamera::startTakingPictures(double intervalSec) {
    // TODO:
    return;
}

bool MockCamera::isDoneTakingPictures() {
    return false;
};