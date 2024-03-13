#include <gtest/gtest.h>

#include "cv/localization.hpp"


void assertLocalizationAccuracy(
    const GPSCoord& expectedCoord,
    const GPSCoord& predictedCoord,
    // TODO: maybe we could specify the threshold in feet (or have a function to convert feet to relative lat/lon)
    double latitudeDegThreshold = 0.00005,
    double longitudeDegThreshold = 0.00005) { 

    ASSERT_NEAR(expectedCoord.latitude(), predictedCoord.latitude(), latitudeDegThreshold);
    ASSERT_NEAR(expectedCoord.longitude(), predictedCoord.longitude(), longitudeDegThreshold);
}


TEST(CVLocalization, LocalizationAccuracy) {
    struct TestCase {
        std::string name;

        ImageTelemetry inputImageTelemetry;
        Bbox inputTargetBbox;

        GPSCoord expectedTargetCoord;
    };

    const std::vector<TestCase> testCases{{
        {
            "directly underneath",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(1951, 1483, 1951, 1483),
            makeGPSCoord(0, 0, 0),
        }, 

        {
            "Blender Case 1",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(4539, 3372, 4539, 3372),
            makeGPSCoord(-0.00002457511350215088, 0.00002863133914919689, 0),
        },

        {
            "Blender Case 2",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(2590, 1702, 2590, 1702),
            makeGPSCoord(0.00000192247925447489, -0.00000231662742315047, 0),      
        },

        {
            "Blender Case 3",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(692, 814, 692, 814),
            makeGPSCoord(0.00001601503082197966, -0.00003243918011301731, 0),
        },

        {
            "Blender Case 4",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(956, 2980, 956, 2980),
            makeGPSCoord(-0.00001836226633958357, -0.00002825011114503169, 0),
        },

        {
            "Blender Case 5",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(3318, 1776, 3318, 1776),
            makeGPSCoord(0.00000075046054290061, 0.00000924642417102570, 0),
        },

        {
            "Blender Case 6",
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0, 0),
            Bbox(3499, 2008, 3499, 2008),
            makeGPSCoord(-0.00000292339877027894, 0.00001211822682158354, 0),
        }
        // TODO: move blender localization test cases here
        // {
        //     "another test case",
        //     ImageTelemetry(0, 0, 100, 50, 0, 0, 0),
        //     Bbox(1951, 1483, 1951, 1483),
        //     makeGPSCoord(0, 0, 0),
        // }
    }};

    for (const auto &testCase : testCases) {
        ECEFLocalization ecefLocalizer;
        GSDLocalization gsdLocalization;

        GPSCoord ecefTargetCoord = ecefLocalizer.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        assertLocalizationAccuracy(testCase.expectedTargetCoord, ecefTargetCoord);

        GPSCoord gsdTargetCoord = gsdLocalization.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        assertLocalizationAccuracy(testCase.expectedTargetCoord, gsdTargetCoord);
    };
}
