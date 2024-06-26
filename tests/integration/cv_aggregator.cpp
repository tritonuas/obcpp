#include <iostream>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"
#include "cv/aggregator.hpp"

// Download these test images from one of the zips here https://drive.google.com/drive/u/1/folders/1opXBWx6bBF7J3-JSFtrhfFIkYis9qhGR
// Or, any cropped not-stolen images will work

// this image should be located at a relative path to the CMake build dir
const std::string imageTestDir = "../tests/integration/images/matching_cropped/test/";
const std::string refImagePath0 = imageTestDir + "000000910.jpg"; // bottle 4
const std::string refImagePath1 = imageTestDir + "000000920.jpg"; // bottle 3
const std::string refImagePath2 = imageTestDir + "000000003.jpg"; // bottle 2
const std::string refImagePath3 = imageTestDir + "000000004.jpg"; // bottle 1
const std::string refImagePath4 = imageTestDir + "000000005.jpg"; // bottle 0

// matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
const std::string matchingModelPath = "../models/target_siamese_1.pt";
// segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
const std::string segmentationModelPath = "../models/fcn.pth";

// mock telemetry data
double latitude = 38.31568;
double longitude = 76.55006;
double altitude = 75;
double airspeed = 20;
double yaw = 100;
double pitch = 5;
double roll = 3;

// integration test to test all stages of the CV pipeline
// with an arbitrary image as input
int main() {
    cv::Mat image = cv::imread(imageTestDir + "000000008.jpg");
    assert(!image.empty());
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed,
        yaw, pitch, roll);
    ImageData imageData("000000008.jpg", imageTestDir, image, mockTelemetry);

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

    // Same setup as cv_pipeline unit test up until this point

    std::cout << "about to create aggregator\n";

    CVAggregator aggregator(pipeline);

    std::cout << "about to run aggregator\n";

    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);
    aggregator.runPipeline(imageData);

    sleep(100000);
}