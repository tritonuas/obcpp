#include <gtest/gtest.h>

#include "cv/utilities.hpp"

#include <loguru.hpp>

TEST(CVUtilities, CropValidAndInvalidSizes) {
    struct TestCase {
        std::string name;
        cv::Mat fullSizeImg;
        Bbox bbox;
        cv::Mat expectedCroppedImg;
        std::string expectedExceptionMsg;
    };

    const std::vector<TestCase> testCases{{
        {
            "basic crop to 100x100",
            // Note that opencv Mat contructor takes (rows, cols, type)
            cv::Mat(1080, 1920, CV_8UC3),
            Bbox{0, 0, 100, 100},
            cv::Mat(100, 100, CV_8UC3),
            ""
        },
        {
            "no crop",
            cv::Mat(1080, 1920, CV_8UC3),
            Bbox{0, 0, 1920, 1080},
            cv::Mat(1080, 1920, CV_8UC3),
            ""
        },
        {
            "crop to 0x0",
            cv::Mat(1080, 1920, CV_8UC3),
            Bbox{0, 0, 0, 0},
            cv::Mat(0, 0, CV_8UC3),
            ""
        },
        {
            "bbox larger than original image",
            cv::Mat(50, 50, CV_8UC3),
            Bbox{0, 0, 100, 150},
            cv::Mat(0, 0, CV_8UC3),
            // idk if it's best to do an equality check on the exception message.
            // maybe it would be enough to check if an exception is thrown or not
            "OpenCV(4.5.4) ./modules/core/src/matrix.cpp:810: error: (-215:Assertion failed) 0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= m.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= m.rows in function 'Mat'\n"
        },
    }};

    for (const auto &testCase : testCases) {
        LOG_F(INFO, "Test name: %s\n", testCase.name.c_str());

        cv::Mat cropped;
        try {
            cropped = crop(testCase.fullSizeImg, testCase.bbox);
        } catch(std::exception& e) {
            EXPECT_EQ(testCase.expectedExceptionMsg, e.what());
            continue;
        }

        EXPECT_EQ(cropped.rows, testCase.expectedCroppedImg.rows);
        EXPECT_EQ(cropped.cols, testCase.expectedCroppedImg.cols);
    }
}
