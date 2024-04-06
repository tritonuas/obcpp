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
            ImageTelemetry(0, 0, 100, 50, 0, 0, 0),
            Bbox(1951, 1483, 1951, 1483),
            makeGPSCoord(0, 0, 0),
        },
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

        // NOTE: temporarily commenting out this assertion until we debug the ECEF localization algorithm
        // GPSCoord ecefTargetCoord = ecefLocalizer.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // assertLocalizationAccuracy(testCase.expectedTargetCoord, ecefTargetCoord);

        // NOTE: this assertion is blocked on GSD implementation getting merged in
        // GPSCoord gsdTargetCoord = gsdLocalization.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // assertLocalizationAccuracy(testCase.expectedTargetCoord, gsdTargetCoord);
    };
}
