#ifndef INCLUDE_CV_PIPELINE_HPP_
#define INCLUDE_CV_PIPELINE_HPP_

#include <vector>
#include <utility>
#include <string>

#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "cv/classification.hpp"
#include "cv/localization.hpp"
#include "cv/matching.hpp"
#include "cv/saliency.hpp"
#include "cv/segmentation.hpp"
#include "cv/utilities.hpp"

// Processed image holds all predictions made concerning a given image.
//
// A single aerial image can have multiple targets that are matches with
// competition bottle assignments.
// At the same time, an aerial image can have other targets that are not matched
// with a bottle (or at least the pipeline is not confident in a match).
struct PipelineResults {
    PipelineResults(ImageData imageData, std::vector<DetectedTarget> targets)
        : imageData{imageData}, targets{targets} {}
    
    ImageData imageData;
    std::vector<DetectedTarget> targets;
};

struct PipelineParams {
    PipelineParams(std::array<Bottle, NUM_AIRDROP_BOTTLES> competitionObjectives,
        std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages,
        std::string matchingModelPath, std::string segmentationModelPath)
        : competitionObjectives{competitionObjectives}, referenceImages{referenceImages},
          matchingModelPath{matchingModelPath}, segmentationModelPath{segmentationModelPath} {}

    std::array<Bottle, NUM_AIRDROP_BOTTLES> competitionObjectives;
    std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages;
    std::string matchingModelPath;
    std::string segmentationModelPath;
};

// Pipeline handles all infrastructure within the CV pipeline
class Pipeline {
 public:
    Pipeline(const PipelineParams& p);

    PipelineResults run(const ImageData& imageData);

 private:
    Saliency detector;

    Matching matcher;

    Segmentation segmentor;
    Classification classifier;

    ECEFLocalization ecefLocalizer;
    GSDLocalization gsdLocalizer;
};

#endif  // INCLUDE_CV_PIPELINE_HPP_
