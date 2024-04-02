#include <iostream>

#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "protos/obc.pb.h"

#include "cv/matching.hpp"
#include "utilities/constants.hpp"

// Download these test images by running "make pull_matching_test_images" or from the test.zip here https://drive.google.com/drive/u/1/folders/1opXBWx6bBF7J3-JSFtrhfFIkYis9qhGR
// Or, any cropped not-stolen images will work
// NOTE: images are given reverse order bottleIndexes, e.g. refImagePath0 -> index 4, etc.
const std::string imageTestDir = "../tests/integration/images/matching_cropped/test/";
const std::string refImagePath0 = imageTestDir + "000000910.jpg"; // bottle 4
const std::string refImagePath1 = imageTestDir + "000000920.jpg"; // bottle 3
const std::string refImagePath2 = imageTestDir + "000000003.jpg"; // bottle 2
const std::string refImagePath3 = imageTestDir + "000000004.jpg"; // bottle 1
const std::string refImagePath4 = imageTestDir + "000000005.jpg"; // bottle 0

// model can be downloaded by running "make pull_matching" or from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
const std::string modelPath = "../models/target_siamese_1.pt";

// These images can also come from the same source as the reference images. To accurately
// run this test, provide one image that is a positive match and one that doesn't match
// any of the references.
const std::string imageMatchPath = imageTestDir + "000000920.jpg";
const int matchIndex = 3; 
const std::string imageNotMatchPath = imageTestDir + "000000016.jpg";

int main(int argc, char* argv[]) {
    // purely for the constructor, doesn't do much in matching
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
    referenceImages.push_back(std::make_pair(ref0, BottleDropIndex(4)));
    cv::Mat ref1 = cv::imread(refImagePath1);
    referenceImages.push_back(std::make_pair(ref1, BottleDropIndex(3)));
    cv::Mat ref2 = cv::imread(refImagePath2);
    referenceImages.push_back(std::make_pair(ref2, BottleDropIndex(2)));
    cv::Mat ref3 = cv::imread(refImagePath3);
    referenceImages.push_back(std::make_pair(ref3, BottleDropIndex(1)));
    cv::Mat ref4 = cv::imread(refImagePath4);
    referenceImages.push_back(std::make_pair(ref4, BottleDropIndex(0)));

    Matching matcher(bottlesToDrop, 0.5, referenceImages, modelPath);
    cv::Mat image = cv::imread(imageMatchPath);
    Bbox dummyBox(0, 0, 0, 0);
    CroppedTarget cropped = {
        image,
        dummyBox,
        false
    };

    cv::Mat falseImage = cv::imread(imageNotMatchPath);
    CroppedTarget croppedFalse = {
        falseImage,
        dummyBox,
        false
    };

    MatchResult result = matcher.match(cropped);
    LOG_F(INFO, "\nTRUE MATCH TEST:\nClosest is bottle at index %d\nfoundMatch is %d\nThe similarity is %.3f\n",
        int(result.bottleDropIndex),
        result.foundMatch,
        result.similarity);

    MatchResult resultFalse = matcher.match(croppedFalse);
    LOG_F(INFO, "\nFALSE MATCH TEST:\nClosest is bottle at index %d\nfoundMatch is %d\nThe similarity is %.3f\n",
        int(resultFalse.bottleDropIndex),
        resultFalse.foundMatch,
        resultFalse.similarity);

    return 0;
}