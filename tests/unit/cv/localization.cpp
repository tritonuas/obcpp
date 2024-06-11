#include <gtest/gtest.h>

#include "cv/localization.hpp"

void assertLocalizationAccuracy(
    const GPSCoord& expectedCoord,
    const GPSCoord& predictedCoord,
    // TODO: should specify the threshold in feet (or have a function to convert feet to relative lat/lon)
    // lat/long is a different amount of distance at different parts on Earth
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
            "real image distance test 1717352989.jpg top target",
            ImageTelemetry(32.9908692, -117.1282454, 31.619001388549805,
                            7.618272304534912, 0.0, 0.0, 0.0, 0.0),
            Bbox(2080, 50, 2280, 240),
            makeGPSCoord(32.9908692, -117.1282454, 0),
        }
    }};

    for (const auto &testCase : testCases) {
        ECEFLocalization ecefLocalizer;
        GSDLocalization gsdLocalization;
        std::cout << "Test case: " << testCase.name << std::endl;

        // GPSCoord ecefTargetCoord = ecefLocalizer.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // assertLocalizationAccuracy(testCase.expectedTargetCoord, ecefTargetCoord);

        GPSCoord gsdTargetCoord = gsdLocalization.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        std::cout << "Calculation error: " << gsdLocalization.distanceInMetersBetweenCords(
            (testCase.expectedTargetCoord.latitude()),
            (testCase.expectedTargetCoord.longitude()),
            (gsdTargetCoord.latitude()),
            (gsdTargetCoord.longitude())) * METER_TO_FT
            << " feet" << std::endl;
        
        assertLocalizationAccuracy(testCase.expectedTargetCoord, gsdTargetCoord);
    };
}
