#ifndef INCLUDE_CV_PIPELINE_HPP_
#define INCLUDE_CV_PIPELINE_HPP_

#include <vector>

#include <opencv2/opencv.hpp>

#include "cv/saliency.hpp"
#include "cv/segmentation.hpp"
#include "cv/classification.hpp"
#include "cv/matching.hpp"
#include "cv/localization.hpp"

#include "camera/interface.hpp"

// Same TODO as above
struct AirdropTarget {
    uint8_t bottleDropIndex;
    GPSCoord coordinate;
};

// Processed image holds all predictions made concerning a given image.
//
// A single aerial image can have multiple targets that are matches with
// competition bottle assignments.
// At the same time, an aerial image can have other targets that are not matched
// with a bottle (or at least the pipeline is not confident in a match).
struct PipelineResults {
    ImageData imageData;
    std::vector<AirdropTarget> matchedTargets;
    // Not sure if unmatchedTargets should hold a different struct than 
    // matchedTargets. Both have basically the same info except unmatched won't
    // have a bottle index assigned to it. We could populate bottle index to -1
    // or leave it as the bottle of the target it was closest in similarity to.
    std::vector<AirdropTarget> unmatchedTargets;
};

// Pipeline handles all infrastructure within the CV pipeline
class Pipeline {
    public:
        Pipeline(std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> 
            competitionObjectives);

        PipelineResults run(const ImageData& imageData);
    private:
        Saliency detector;

        Matching matcher;

        Segmentation segmentor;
        Classification classifier; 

        Localization localizer;
};

#endif  // INCLUDE_CV_PIPELINE_HPP_
