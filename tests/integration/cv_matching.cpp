#include <iostream>

#include <opencv2/opencv.hpp>

#include "protos/obc.pb.h"

#include "cv/matching.hpp"
#include "utilities/constants.hpp"

const std::string refImagePath0 = "../bin/test/test/000000910.jpg"; // bottle 4
const std::string refImagePath1 = "../bin/test/test/000000920.jpg"; // bottle 3
const std::string refImagePath2 = "../bin/test/test/000000003.jpg"; // bottle 2
const std::string refImagePath3 = "../bin/test/test/000000004.jpg"; // bottle 1
const std::string refImagePath4 = "../bin/test/test/000000005.jpg"; // bottle 0
// Note: images are given reverse order bottleIndexes, e.g. refImagePath0 -> index 4, etc.
const std::string modelPath = "../bin/target_siamese_1.pt";
const std::string imageMatchPath = "../bin/test/test/000000920.jpg";
const int matchIndex = 3; 
const std::string imageNotMatchPath = "../bin/test/test/000000016.jpg";

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
    std::cout << "TRUE MATCH TEST:" << std::endl;
    std::cout << "Found a match with bottle at index " << int(result.bottleDropIndex) << std::endl;
    std::cout << "Expected bottle " << matchIndex << std::endl;
    std::cout << "foundMatch is " << result.foundMatch << std::endl;
    std::cout << "The similarity is " << result.similarity << std::endl;


    MatchResult resultFalse = matcher.match(croppedFalse);
    std::cout << "\nFALSE MATCH TEST:" << std::endl;
    std::cout << "Closest is bottle at index " << int(resultFalse.bottleDropIndex) << std::endl;
    std::cout << "foundMatch is " << resultFalse.foundMatch << std::endl;
    std::cout << "The similarity is " << resultFalse.similarity << std::endl;

    return 0;
}