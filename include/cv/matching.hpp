#ifndef INCLUDE_CV_MATCHING_HPP_
#define INCLUDE_CV_MATCHING_HPP_

#include <opencv2/opencv.hpp>
#include "cv/utilities.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include <torch/torch.h>
#include <torch/script.h>
#include <torchvision/vision.h>

#include "protos/obc.pb.h"

struct MatchResult {
    uint8_t bottleDropIndex;
    bool foundMatch;
    double similarity;
};

// Matching is used to match targets to a potential corresponding competition
// bottle drop objective. Before the misssion starts, the competition tells us
// the characteristics of the ground targets that we should drop bottles on.
// Using this information we know which targets to look for. Then, we can
// calculate how similar the targets we capture are to the ones the competition
// enumerates for the bottle drop.
// This is implemented using a Siamese Neural Netowrk that takes two images as
// input and outputs a similarity score. One of the input images will be the
// cropped target from saliency. The other input will be an artificially
// generated image that matches the description of one of the competition
// objectives. We will use something similar to not-stolen-israeli-code to do
// the generation (https://github.com/tritonuas/not-stolen-israeli-code).
// The matching siamese model is implemented in this repo:
// https://github.com/tritonuas/fraternal-targets
//
// One important note is that matching is a replacement for the segmentation
// and classification stages of the pipeline. Instead of outright guessing
// the properties of the targets (as with segmentation/classification) we can
// see how close it is to known objectives.
class Matching {
 public:
        Matching(std::array<Bottle, NUM_AIRDROP_BOTTLES> competitionObjectives, 
            double matchThreshold, 
            std::vector<std::pair<cv::Mat, uint8_t>> referenceImages,
            const std::string &modelPath);

        MatchResult match(const CroppedTarget& croppedTarget);

 private:
        std::array<Bottle, NUM_AIRDROP_BOTTLES> competitionObjectives;
        double matchThreshold;
        std::vector<std::pair<torch::Tensor, uint8_t>> referenceFeatures;
        torch::jit::script::Module module;
};

#endif  // INCLUDE_CV_MATCHING_HPP_
