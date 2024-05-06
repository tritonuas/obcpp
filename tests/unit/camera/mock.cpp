#include <gtest/gtest.h>

#include "camera/interface.hpp"
#include "camera/mock.hpp"

// test that the mock camera returns a valid image
TEST(MockCamera, TakePicture) {
    CameraConfig config;
    config.type = "mock";
    MockCamera camera(config);

    camera.connect();

    camera.startTakingPictures();
    ImageData image = camera.getLastPicture();
    camera.stopTakingPictures();

    EXPECT_EQ(image.getData().size(), cv::Size(4000, 3000));
}