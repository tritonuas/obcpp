#include <iostream>

#include <opencv2/opencv.hpp>

#include "cv/pipeline.hpp"

// this image should be located at a relative path to the CMake build dir
const std::string imagePath = "mock_image.jpg";

// mock telemetry data
const double latitude = 38.31568;
const double longitude = 76.55006;
const double altitude = 75;
const double airspeed = 20;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

int main() {
    cv::Mat image = cv::imread(imagePath);
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed,
        yaw, pitch, roll);
    ImageData imageData("mock_image", imagePath, image, mockTelemetry);

    std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottlesToDrop = {
        CompetitionBottle{
            ODLCColor::Black,
            ODLCShape::Circle,
            ODLCColor::Blue,
            'C'
        },
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Triangle,
            ODLCColor::Green,
            'X'
        },
        CompetitionBottle{
            ODLCColor::Purple,
            ODLCShape::Semicircle,
            ODLCColor::Orange,
            'T'
        },
        CompetitionBottle{
            ODLCColor::White,
            ODLCShape::Pentagon,
            ODLCColor::Red,
            'Z'
        },
        CompetitionBottle{
            ODLCColor::Blue,
            ODLCShape::QuarterCircle,
            ODLCColor::Brown,
            'B'
        },
    };

    Pipeline pipeline(bottlesToDrop);

    PipelineResults output = pipeline.run(imageData);

    size_t numTargets = output.matchedTargets.size() +
        output.unmatchedTargets.size();
    size_t numMatches = output.matchedTargets.size();

    std::cout << "Found " << numTargets << " targets" << std::endl;
    std::cout << "Found " << numMatches << " matches" << std::endl;

    for (AirdropTarget& match: output.matchedTargets) {
        std::cout << "Found match assigned to bottle index " << 
            match.bottleDropIndex << std::endl;
    }
}