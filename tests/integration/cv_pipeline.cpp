#include <iostream>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"


// NOTE: all paths to images should be located at a relative path to the CMake build dir.
// You can download all images with `ninja pull_test_images`

// Download by running `ninja pull_saliency_test_images` or from this drive folder 
// https://drive.google.com/drive/folders/1JvtQUroZJHo51E37_IA2D1mfdJj2smyR?usp=drive_link
const std::string imagePath = "../tests/integration/images/saliency/1718243323.jpg";

// Download these test images from here https://drive.google.com/drive/folders/1vmP3HUS1SyqhdtJrP4QuFpGbyoyfaYSe?usp=drive_link
// Or, any cropped not-stolen images will work

#define NUM_MATCHING_IMAGES 20
// Directories to matching images. Note that each element of this array is a std::pair.
// The first member is the path to the image and the second member is the bottle each matching
// image corresponds to. See the body of main() for what shape/char/color each bottle is assigned to.
const std::array<std::pair<std::string, BottleDropIndex>, NUM_MATCHING_IMAGES> matchingImgPaths = {
    std::make_pair("../tests/integration/images/matching/green_rect_purple_B_1.jpg", BottleDropIndex::B),
    std::make_pair("../tests/integration/images/matching/green_rect_purple_B_2.jpg", BottleDropIndex::B),
    std::make_pair("../tests/integration/images/matching/green_rect_purple_B_3.jpg", BottleDropIndex::B),
    std::make_pair("../tests/integration/images/matching/green_rect_purple_B_4.jpg", BottleDropIndex::B),
    std::make_pair("../tests/integration/images/matching/green_rect_purple_B_5.jpg", BottleDropIndex::B),

    std::make_pair("../tests/integration/images/matching/red_triangle_purple_E_1.jpg", BottleDropIndex::C),
    std::make_pair("../tests/integration/images/matching/red_triangle_purple_E_2.jpg", BottleDropIndex::C),
    std::make_pair("../tests/integration/images/matching/red_triangle_purple_E_3.jpg", BottleDropIndex::C),
    std::make_pair("../tests/integration/images/matching/red_triangle_purple_E_4.jpg", BottleDropIndex::C),
    std::make_pair("../tests/integration/images/matching/red_triangle_purple_E_5.jpg", BottleDropIndex::C),

    std::make_pair("../tests/integration/images/matching/blue_pent_green_Q_1.jpg", BottleDropIndex::D),
    std::make_pair("../tests/integration/images/matching/blue_pent_green_Q_2.jpg", BottleDropIndex::D),
    std::make_pair("../tests/integration/images/matching/blue_pent_green_Q_3.jpg", BottleDropIndex::D),
    std::make_pair("../tests/integration/images/matching/blue_pent_green_Q_4.jpg", BottleDropIndex::D),
    std::make_pair("../tests/integration/images/matching/blue_pent_green_Q_5.jpg", BottleDropIndex::D),

    std::make_pair("../tests/integration/images/matching/blue_pent_orange_3_1.jpg", BottleDropIndex::E),
    std::make_pair("../tests/integration/images/matching/blue_pent_orange_3_2.jpg", BottleDropIndex::E),
    std::make_pair("../tests/integration/images/matching/blue_pent_orange_3_3.jpg", BottleDropIndex::E),
    std::make_pair("../tests/integration/images/matching/blue_pent_orange_3_4.jpg", BottleDropIndex::E),
    std::make_pair("../tests/integration/images/matching/blue_pent_orange_3_5.jpg", BottleDropIndex::E),
};

// matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
const std::string matchingModelPath = "../models/target_siamese_1.pt";
// segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
const std::string segmentationModelPath = "../models/fcn-model_20-epochs_06-01-2023T21-16-02.pth";
const std::string saliencyModelPath = "../models/torchscript_19.pth";

// mock telemetry data
const double latitude = 32.990795399999996;
const double longitude = -117.1282463;
const double altitude = 30.108001708984375;
const double airspeed = 7.378872394561768;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

// integration test to test all stages of the CV pipeline
// with an arbitrary image as input
int main() {
    cv::Mat image = cv::imread(imagePath);
    if (!image.data) {
        std::cout << "failed to open testing image from " << imagePath << std::endl;
        return 1;
    }
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed,
        yaw, pitch, roll);
    ImageData imageData(image, 0, mockTelemetry);

    std::array<Bottle, NUM_AIRDROP_BOTTLES> bottlesToDrop;

    // Bottle A
    Bottle bottle1;
    bottle1.set_ismannikin(true);
    bottlesToDrop[0] = bottle1;

    // Bottle B
    Bottle bottle2;
    bottle2.set_shapecolor(ODLCColor::Green);
    bottle2.set_shape(ODLCShape::Rectangle);
    bottle2.set_alphanumericcolor(ODLCColor::Purple);
    bottle2.set_alphanumeric("B");
    bottlesToDrop[1] = bottle2;

    // Bottle C
    Bottle bottle3;
    bottle3.set_shapecolor(ODLCColor::Red);
    bottle3.set_shape(ODLCShape::Triangle);
    bottle3.set_alphanumericcolor(ODLCColor::Purple);
    bottle3.set_alphanumeric("E");
    bottlesToDrop[2] = bottle3;

    // Bottle D
    Bottle bottle4;
    bottle4.set_shapecolor(ODLCColor::Blue);
    bottle4.set_shape(ODLCShape::Pentagon);
    bottle4.set_alphanumericcolor(ODLCColor::Green);
    bottle4.set_alphanumeric("Q");
    bottlesToDrop[3] = bottle4;

    // Bottle E
    Bottle bottle5;
    bottle5.set_shapecolor(ODLCColor::Blue);
    bottle5.set_shape(ODLCShape::Pentagon);
    bottle5.set_alphanumericcolor(ODLCColor::Orange);
    bottle5.set_alphanumeric("3");
    bottlesToDrop[4] = bottle5;

    std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages;
    for (const auto& [matchingImgPath, assignedBottle]: matchingImgPaths) {
        cv::Mat refImg = cv::imread(matchingImgPath);
        referenceImages.push_back(std::make_pair(refImg, assignedBottle));
    }

    Pipeline pipeline(PipelineParams(bottlesToDrop, referenceImages, matchingModelPath, segmentationModelPath, saliencyModelPath));

    PipelineResults output = pipeline.run(imageData);

    size_t numTargets = output.targets.size();

    LOG_F(INFO, "Detected %ld targets", numTargets);

    for (DetectedTarget& t: output.targets) {
        LOG_F(INFO, "Detected Bottle %d at (%f %f) with match distance %f \n",
            t.likely_bottle, t.coord.latitude(), t.coord.longitude(), t.match_distance);
    }
}