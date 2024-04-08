#include <iostream>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"

// Download these test images from one of the zips here https://drive.google.com/drive/u/1/folders/1opXBWx6bBF7J3-JSFtrhfFIkYis9qhGR
// Or, any cropped not-stolen images will work

// this image should be located at a relative path to the CMake build dir
const std::string imagePath = "mock_image.jpg";

const std::string refImagePath0 = "../bin/test/test/000000910.jpg";
const std::string refImagePath1 = "../bin/test/test/000000920.jpg";
const std::string refImagePath2 = "../bin/test/test/000000003.jpg";
const std::string refImagePath3 = "../bin/test/test/000000004.jpg";
const std::string refImagePath4 = "../bin/test/test/000000005.jpg";

// matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
const std::string matchingModelPath = "../models/target_siamese_1.pt";
// segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
const std::string segmentationModelPath = "../models/fcn.pth";

// mock telemetry data
const double latitude = 38.31568;
const double longitude = 76.55006;
const double altitude = 75;
const double airspeed = 20;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

// integration test to test all stages of the CV pipeline
// with an arbitrary image as input
int main() {
    cv::Mat image = cv::imread(imagePath);
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed,
        yaw, pitch, roll);
    ImageData imageData("mock_image", imagePath, image, mockTelemetry);

    std::array<Bottle, NUM_AIRDROP_BOTTLES> bottlesToDrop;

    Bottle bottle1;
    bottle1.set_shapecolor(ODLCColor::Red);
    bottle1.set_shape(ODLCShape::Circle);
    bottle1.set_alphanumericcolor(ODLCColor::Orange);
    bottle1.set_alphanumeric("J");
    bottlesToDrop[0] = bottle1;

    Bottle bottle2;
    bottle2.set_shapecolor(ODLCColor::Blue);
    bottle2.set_shape(ODLCShape::Circle);
    bottle2.set_alphanumericcolor(ODLCColor::Orange);
    bottle2.set_alphanumeric("G");
    bottlesToDrop[1] = bottle2;

    Bottle bottle3;
    bottle3.set_shapecolor(ODLCColor::Red);
    bottle3.set_shape(ODLCShape::Circle);
    bottle3.set_alphanumericcolor(ODLCColor::Blue);
    bottle3.set_alphanumeric("X");
    bottlesToDrop[2] = bottle3;

    Bottle bottle4;
    bottle4.set_shapecolor(ODLCColor::Red);
    bottle4.set_shape(ODLCShape::Circle);
    bottle4.set_alphanumericcolor(ODLCColor::Blue);
    bottle4.set_alphanumeric("F");
    bottlesToDrop[3] = bottle4;

    Bottle bottle5;
    bottle5.set_shapecolor(ODLCColor::Green);
    bottle5.set_shape(ODLCShape::Circle);
    bottle5.set_alphanumericcolor(ODLCColor::Black);
    bottle5.set_alphanumeric("F");
    bottlesToDrop[4] = bottle5;

    std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages;
    cv::Mat ref0 = cv::imread(refImagePath0);
    referenceImages.push_back(std::make_pair(ref0, BottleDropIndex(5)));
    cv::Mat ref1 = cv::imread(refImagePath1);
    referenceImages.push_back(std::make_pair(ref1, BottleDropIndex(4)));
    cv::Mat ref2 = cv::imread(refImagePath2);
    referenceImages.push_back(std::make_pair(ref2, BottleDropIndex(3)));
    cv::Mat ref3 = cv::imread(refImagePath3);
    referenceImages.push_back(std::make_pair(ref3, BottleDropIndex(2)));
    cv::Mat ref4 = cv::imread(refImagePath4);
    referenceImages.push_back(std::make_pair(ref4, BottleDropIndex(1)));

    Pipeline pipeline(PipelineParams(bottlesToDrop, referenceImages, matchingModelPath, segmentationModelPath));

    PipelineResults output = pipeline.run(imageData);

    size_t numTargets = output.targets.size();

    LOG_F(INFO, "Detected %ld targets", numTargets);

    for (DetectedTarget& t: output.targets) {
        LOG_F(INFO, "Detected Bottle %d at (%f %f) with match distance %f \n",
            t.likely_bottle, t.coord.latitude(), t.coord.longitude(), t.match_distance);
    }
}