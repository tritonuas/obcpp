#include <gtest/gtest.h>

#include "cv/localization.hpp"

void assertLocalizationAccuracy(
    const GPSCoord& expectedCoord,
    const GPSCoord& predictedCoord,
    // TODO: should specify the threshold in feet (or have a function to convert feet to relative lat/lon)
    // lat/long is a different amount of distance at different parts on Earth
    double latitudeDegThreshold = 0.0000005,
    double longitudeDegThreshold = 0.0000005) { 

    SCOPED_TRACE(::testing::Message() << "\nExpected Lat, Long: " << expectedCoord.latitude() << ", "<< expectedCoord.longitude()
                                      << "\nPredicted Lat, Long: " << predictedCoord.latitude() << ", " << predictedCoord.longitude());
        
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
            // sewer cover plate as the target (quadrant 1)
            // image directory: out 5/00000330.jpg, teelmetry directory: 033.csv"
            // IMPORTNAT: TELEMENTRY FROM TEST FLIGHT IMAGES FROM THE BIG QUAD HAS A BIASED HEADING ANGLE, SUBTRACT HDG BY AROUND 25 DEGREES

            "REAL IMAGE TEST FLIGHT 1_MID ALT_1",
            ImageTelemetry(32.8811581, -117.2353253, 45.722,
                            0.0, 207.85, 0.0, 0.0, 0.0),
            Bbox(1291, 664, 1293, 666),
            makeGPSCoord(32.881152, -117.235426, 0),
        },

        {
            // corner of grass as the target (quadrant 4)
            // image directory: out 5/00000330.jpg, teelmetry directory: 033.csv"
            // IMPORTNAT: TELEMENTRY FROM TEST FLIGHT IMAGES FROM THE BIG QUAD HAS A BIASED HEADING ANGLE, SUBTRACT HDG BY AROUND 25 DEGREES

            "REAL IMAGE TEST FLIGHT_MID ALT_2",
            ImageTelemetry(32.8811581, -117.2353253, 45.722,
                            0.0, 207.85, 0.0, 0.0, 0.0),
            Bbox(1030, 1301, 1030, 1301),
            makeGPSCoord(32.881252, -117.235290, 0),
        },
        
        {
            // sewer covre as the target (quadrant 3)
            // image directory: out 5/00000560.jpg, teelmetry directory: 056.csv"
            // IMPORTNAT: TELEMENTRY FROM TEST FLIGHT IMAGES FROM THE BIG QUAD HAS A BIASED HEADING ANGLE, SUBTRACT HDG BY AROUND 25 DEGREES

            "REAL IMAGE TEST FLIGHT 1_HIGH ALT_1",
            ImageTelemetry(32.8811630, -117.2353358, 100.923,
                            0.0, 207.85, 0.0, 0.0, 0.0),
            Bbox(342, 1425, 342, 1425),
            makeGPSCoord(32.881298, -117.234769, 0),
        },

        {
            // corner of grass as the target (quadrant 2)
            // image directory: out 5/00000560.jpg, teelmetry directory: 056.csv"
            // IMPORTNAT: TELEMENTRY FROM TEST FLIGHT IMAGES FROM THE BIG QUAD HAS A BIASED HEADING ANGLE, SUBTRACT HDG BY AROUND 25 DEGREES

            "REAL IMAGE TEST FLIGHT 1_HIGH ALT_2",
            ImageTelemetry(32.8811630, -117.2353358, 100.923,
                            0.0, 207.85, 0.0, 0.0, 0.0),
            Bbox(402, 570, 402, 570),
            makeGPSCoord(32.880980, -117.235094, 0),
        },

        {
            // DUMMY TEST (center of image)
            "DUMMY TEST",
            ImageTelemetry(32.8811581, -117.2353253, 45.722,
                            0.0, 207.85, 0.0, 0.0, 0.0),
            Bbox(1014, 760, 1014, 760),
            makeGPSCoord(32.8811581, -117.2353253, 0),
        },
        
        // {
        //     // sewer cover plate as the target
        //     "test flight 1, out 5, data: 033.csv, image: 00000337.jpg",
        //     ImageTelemetry(32.9908692, -117.1282454, 31.619001388549805,
        //                     7.618272304534912, 0.0, 0.0, 0.0, 0.0),
        //     Bbox(2080, 50, 2280, 240),
        //     makeGPSCoord(32.881152, -117.235547, 0),
        // }

    }};

    for (const auto &testCase : testCases) {
        ECEFLocalization ecefLocalizer;
        GSDLocalization gsdLocalization;
        std::cout << "Test case: " << testCase.name << std::endl;

        // GPSCoord ecefTargetCoord = ecefLocalizer.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // assertLocalizationAccuracy(testCase.expectedTargetCoord, ecefTargetCoord);

        GPSCoord gsdTargetCoord = gsdLocalization.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // GPSCoord gsdTargetCoord = ecefLocalizer.localize(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        std::cout << "Calculation error: " << gsdLocalization.distanceInMetersBetweenCords(
            (testCase.expectedTargetCoord.latitude()),
            (testCase.expectedTargetCoord.longitude()),
            (gsdTargetCoord.latitude()),
            (gsdTargetCoord.longitude())) * METER_TO_FT
            << " feet" << std::endl;


        // double GSDvalue = gsdLocalization.debug(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // std::cout << "GSD value: " << GSDvalue << "mm/px" << std::endl;

        auto result = gsdLocalization.debug(testCase.inputImageTelemetry, testCase.inputTargetBbox);
        // Decompose the result tuple
        double GSD, calc_cam_offset_x_m, calc_cam_offset_y_m;
        std::tie(GSD, calc_cam_offset_x_m, calc_cam_offset_y_m) = result;

        // Print the results
        std::cout << "GSD: " << GSD << " mm/px" << std::endl;
        std::cout << "Camera Offset X: " << calc_cam_offset_x_m << " m" << std::endl;
        std::cout << "Camera Offset Y: " << calc_cam_offset_y_m << " m" << std::endl;

        assertLocalizationAccuracy(testCase.expectedTargetCoord, gsdTargetCoord);
    };
}
