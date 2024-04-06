#include <gtest/gtest.h>

#include "camera/interface.hpp"
#include "camera/mock.hpp"

// test that the mock camera returns a valid image
TEST(MockCamera, TakePicture) {
    CameraConfiguration config({
       {"SampleConfigKey", 100},
       {"ExposureTime", 1000},
    });
    MockCamera camera(config);

    camera.connect();

    camera.takePicture();
    ImageData image = camera.getLastPicture();

    EXPECT_EQ(image.getData().size(), cv::Size(4000, 3000));
}