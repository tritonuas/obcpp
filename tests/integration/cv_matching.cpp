#include "cv/matching.hpp"
#include <iostream>

#include <opencv2/opencv.hpp>

const std::string refImagePath0 = "../bin/test/test/000000910.jpg";
const std::string refImagePath1 = "../bin/test/test/000000920.jpg";
const std::string refImagePath2 = "../bin/test/test/000000003.jpg";
const std::string refImagePath3 = "../bin/test/test/000000004.jpg";
const std::string refImagePath4 = "../bin/test/test/000000005.jpg";
// Note: images are given reverse order bottleIndexes, e.g. refImagePath0 -> index 4, etc.
const std::string modelPath = "../bin/target_siamese_1.pt";
const std::string imageMatchPath = "../bin/test/test/000000920.jpg"; 
// image 1 -> bottleIdx 3
const int bottleIdxMatch = 3;
const std::string imageNotMatchPath = "../bin/test/test/000000016.jpg";

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

    std::vector<std::pair<cv::Mat, uint8_t>> referenceImages;
    cv::Mat ref0 = cv::imread(refImagePath0);
    referenceImages.push_back(std::make_pair(ref0, 4));
    cv::Mat ref1 = cv::imread(refImagePath1);
    referenceImages.push_back(std::make_pair(ref1, 3));
    cv::Mat ref2 = cv::imread(refImagePath2);
    referenceImages.push_back(std::make_pair(ref2, 2));
    cv::Mat ref3 = cv::imread(refImagePath3);
    referenceImages.push_back(std::make_pair(ref3, 1));
    cv::Mat ref4 = cv::imread(refImagePath4);
    referenceImages.push_back(std::make_pair(ref4, 0));

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
    std::cout << "MATCHING TEST:" << std::endl;
    std::cout << "Found a match with bottle " << int(result.bottleDropIndex) << std::endl;
    std::cout << "Expected bottle with index " << bottleIdxMatch << std::endl;
    std::cout << "foundMatch is " << result.foundMatch << std::endl;
    std::cout << "The similarity is " << result.similarity << std::endl;


    MatchResult resultFalse = matcher.match(croppedFalse);
    std::cout << "\nNO MATCH TEST:" << std::endl;
    std::cout << "Closest is bottle with index " << int(resultFalse.bottleDropIndex) << std::endl;
    std::cout << "foundMatch is " << resultFalse.foundMatch << std::endl;
    std::cout << "The similarity is " << resultFalse.similarity << std::endl;

    return 0;
}