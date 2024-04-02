#include <iostream>

#include <opencv2/opencv.hpp>

#include "cv/segmentation.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

// target image paths, modify to use your targets
const std::vector<std::string> targetPaths = {
    "../tests/integration/images/target0.jpg",
    "../tests/integration/images/target1.jpg",
    "../tests/integration/images/target2.jpg",
    "../tests/integration/images/target3.jpg",
    "../tests/integration/images/target4.jpg"
};

// model weights path
// Can pull model weights by running "make pull_segmentation"
const std::string modelPath = "../models/fcn.pth";

int main(int argc, char* argv[]) {
    Segmentation segmentation(modelPath);
    CroppedTarget ct;

    int width = 0;
    int height = 0;
    std::vector<cv::Mat*> targetImgs;
    for (int i = 0; i < targetPaths.size(); i++) {
        auto target = cv::imread(targetPaths[i]);
        ct.croppedImage = target;
        auto result = segmentation.segment(ct);

        cv::Mat characterMask(result.characterMask.size(), result.characterMask.type());
        result.characterMask.copyTo(characterMask);
        cv::Mat shapeMask(result.shapeMask.size(), result.shapeMask.type());
        result.shapeMask.copyTo(shapeMask);

        targetImgs.push_back(new cv::Mat(target));
        targetImgs.push_back(new cv::Mat(characterMask));
        targetImgs.push_back(new cv::Mat(shapeMask));

        height += target.rows;
        if (width < target.cols) {
            width = target.cols;
        }
    }
    
    cv::Mat canvas_input = cv::Mat::zeros(height, width, targetImgs[0]->type());
    cv::Mat canvas_masks = cv::Mat::zeros(height, width*2, targetImgs[1]->type());

    int j = 0;
    for (int i = 0; i < targetImgs.size(); i+=3) {
        targetImgs[i]->copyTo(canvas_input(cv::Rect(cv::Point(0, j), targetImgs[i]->size())));
        targetImgs[i+1]->copyTo(canvas_masks(cv::Rect(cv::Point(0, j), targetImgs[i+1]->size())));
        targetImgs[i+2]->copyTo(canvas_masks(cv::Rect(cv::Point(targetImgs[i+1]->cols, j), targetImgs[i+2]->size())));
        
        j += targetImgs[i]->rows;

        delete targetImgs[i];
        delete targetImgs[i+1];
        delete targetImgs[i+2];
    }

    cv::namedWindow("targets");
    cv::imshow("targets", canvas_input);
    cv::namedWindow("character masks, shape masks");
    cv::imshow("character masks, shape masks", canvas_masks);
    cv::waitKey(0);
    
    return 0;
}