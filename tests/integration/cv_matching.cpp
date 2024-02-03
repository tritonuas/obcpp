#include "cv/matching.hpp"
#include <iostream>

#include <opencv2/opencv.hpp>

const std::string refImagePath0 = "../bin/test/test/000000000.jpg";
const std::string refImagePath1 = "../bin/test/test/000000001.jpg";
const std::string refImagePath2 = "../bin/test/test/000000002.jpg";
const std::string refImagePath3 = "../bin/test/test/000000003.jpg";
const std::string refImagePath4 = "../bin/test/test/000000004.jpg";

const std::string modelPath = "../bin/target_siamese_1.pt";
const std::string imageMatchPath = "../bin/test/test/000000003.jpg";
const std::string imageNotMatchPath = "../bin/test/test/000000005.jpg";

int main(int argc, char* argv[]) {
    // purely for the constructor, doesn't do much in matching
    std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottlesToDrop = {
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Circle,
            ODLCColor::Orange,
            'J'
        },
        CompetitionBottle{
            ODLCColor::Blue,
            ODLCShape::Circle,
            ODLCColor::Orange,
            'G'
        },
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Circle,
            ODLCColor::Blue,
            'X'
        },
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Circle,
            ODLCColor::Blue,
            'F'
        },
        CompetitionBottle{
            ODLCColor::Green,
            ODLCShape::Circle,
            ODLCColor::Black,
            'F'
        },
    };

    std::vector<cv::Mat> referenceImages;
    cv::Mat ref0 = cv::imread(refImagePath0);
    referenceImages.push_back(ref0);
    cv::Mat ref1 = cv::imread(refImagePath1);
    referenceImages.push_back(ref1);
    cv::Mat ref2 = cv::imread(refImagePath2);
    referenceImages.push_back(ref2);
    cv::Mat ref3 = cv::imread(refImagePath3);
    referenceImages.push_back(ref3);
    cv::Mat ref4 = cv::imread(refImagePath4);
    referenceImages.push_back(ref4);

    Matching matcher(bottlesToDrop, 0.5, referenceImages, modelPath);
    cv::Mat image = cv::imread(imageMatchPath);
    Bbox dummyBox(0, 0, 0, 0);
    CroppedTarget cropped = {
        image,
        dummyBox,
        false
    };

    MatchResult result = matcher.match(cropped);
    std::cout << "Found a match with bottle character " << result.bottleDropIndex << std::endl;
    std::cout << "foundMatch is " << result.foundMatch << std::endl;
    std::cout << "The similarity is " << result.similarity << std::endl;

    return 0;
}