#include "cv/utilities.hpp"

#include <gtest/gtest.h>

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
        {"basic crop to 100x100",
         // Note that opencv Mat contructor takes (rows, cols, type)
         cv::Mat(1080, 1920, CV_8UC3), Bbox{0, 0, 100, 100}, cv::Mat(100, 100, CV_8UC3), ""},
        {"no crop", cv::Mat(1080, 1920, CV_8UC3), Bbox{0, 0, 1920, 1080},
         cv::Mat(1080, 1920, CV_8UC3), ""},
        {"crop to 0x0", cv::Mat(1080, 1920, CV_8UC3), Bbox{0, 0, 0, 0}, cv::Mat(0, 0, CV_8UC3), ""},
        {"bbox larger than original image", cv::Mat(50, 50, CV_8UC3), Bbox{0, 0, 100, 150},
         cv::Mat(0, 0, CV_8UC3),
         "OpenCV(4.5.4) ./modules/core/src/matrix.cpp:810: error: (-215:Assertion failed) 0 <= "
         "roi.x && 0 <= roi.width && roi.x + roi.width <= m.cols && 0 <= roi.y && 0 <= roi.height "
         "&& roi.y + roi.height <= m.rows in function 'Mat'\n"},
    }};

    for (const auto& testCase : testCases) {
        LOG_F(INFO, "Test name: %s\n", testCase.name.c_str());

        cv::Mat cropped;
        try {
            cropped = crop(testCase.fullSizeImg, testCase.bbox);

            // Only perform size checks if no exception was expected
            if (testCase.expectedExceptionMsg.empty()) {
                EXPECT_EQ(cropped.rows, testCase.expectedCroppedImg.rows)
                    << "Test case '" << testCase.name << "' failed: rows mismatch";
                EXPECT_EQ(cropped.cols, testCase.expectedCroppedImg.cols)
                    << "Test case '" << testCase.name << "' failed: cols mismatch";

                // Additional validation for non-empty images
                if (testCase.expectedCroppedImg.rows > 0 && testCase.expectedCroppedImg.cols > 0) {
                    EXPECT_EQ(cropped.type(), testCase.expectedCroppedImg.type())
                        << "Test case '" << testCase.name << "' failed: type mismatch";
                }
            }
        } catch (const std::exception& e) {
            // If we expected an exception, verify the message
            if (!testCase.expectedExceptionMsg.empty()) {
                EXPECT_EQ(testCase.expectedExceptionMsg, e.what())
                    << "Test case '" << testCase.name << "' failed: unexpected exception message";
            } else {
                FAIL() << "Test case '" << testCase.name
                       << "' failed: unexpected exception: " << e.what();
            }
        }
    }
}
