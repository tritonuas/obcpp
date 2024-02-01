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
const std::string imageNotMatchPath = "../bin/test/test/000000005.jpg"

int main(int argc, char* argv[]) {
    // purely for the constructor, doesn't do much in matching
    std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottlesToDrop = {
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Hexagon,
            ODLCColor::Orange,
            'J'
        },
        CompetitionBottle{
            ODLCColor::Blue,
            ODLCShape::Square,
            ODLCColor::Orange,
            'G'
        },
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Square,
            ODLCColor::Blue,
            'X'
        },
        CompetitionBottle{
            ODLCColor::Red,
            ODLCShape::Square,
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
    for(int i = 0; i < 5; i++) {
        cv::Mat image = cv::imread("../bin/test/test/00000000" + i + ".jpg");
        referenceImages.push_back(image);
    }

    Matching matcher(bottlesToDrop, 0.5, referenceImages, modelPath);
    cv::Mat image = cv::imread(imageMatchPath);
    BBox dummyBox(0, 0, 0, 0);
    CroppedTarget cropped = {
        image,
        dummyBox,
        false
    };

    MatchResult result = matcher.match(&cropped);
    std::cout << "Found a match with bottle index " << result.bottleDropIndex << std::endl;
    std::cout << "foundMatch is " << result.foundMatch << std::endl;
    std::cout << "The similarity is " << result.similarity << std::endl;

    return 0;
}