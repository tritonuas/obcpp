#include "cv/pipeline.hpp"

const double DEFAULT_MATCHING_THRESHOLD = 0.5;
const std::string modelPath = "../bin/target_siamese_1.pt";

// TODO: eventually we will need to invoke non-default constructors for all the
// modules of the pipeline (to customize model filepath, etc...)
// TODO: I also want to have a way to customize if the model will use
// matching vs segmentation/classification
Pipeline::Pipeline(std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES>
    competitionObjectives, std::vector<std::pair<cv::Mat, uint8_t>> referenceImages) : 
    // assumes reference images passed to pipeline from not_stolen
        matcher(competitionObjectives, DEFAULT_MATCHING_THRESHOLD, referenceImages, modelPath) {}

/*
 *  Entrypoint of CV Pipeline. At a high level, it will include the following
 *  stages:
 *      - Saliency takes the full size image and outputs any cropped targets
 *        that are detected
 *      - Cropped targets are passed into target matching where they will
 *        be compared against targets associated with each water bottle.
 *        All targets (regardless of match status) will be passed onto
 *        later stages.
 *      - Bounding boxes predicted from saliency and GPS telemetry from when
 *        the photo was captured are passed into the localization algorithm.
 *        Here, the real latitude/longitude coordinates of the target are
 *        calculated.
 *      - Note, we may use a slight variation of this pipeline where the
 *        target matching stage is replaced by segmentation/classification.
 */
PipelineResults Pipeline::run(const ImageData &imageData) {
    std::vector<CroppedTarget> saliencyResults = detector.salience(imageData.getData());

    // if saliency finds no potential targets, end early with no results
    if (saliencyResults.empty()) {
        return PipelineResults{imageData, {}, {}};
    }

    // go through saliency results and determine if each potential target
    // matches one of the targets assigned to a bottle for a competition
    /// objective
    std::vector<AirdropTarget> matchedTargets;
    std::vector<AirdropTarget> unmatchedTargets;
    for (CroppedTarget target : saliencyResults) {
        MatchResult potentialMatch = matcher.match(target);

        GPSCoord targetPosition = localizer.localize(imageData.getTelemetry(), target.bbox);

        AirdropTarget airdropTarget = {potentialMatch.bottleDropIndex, targetPosition};

        // TODO: we should have some criteria to handle duplicate targets that
        // show up in multiple images
        if (potentialMatch.foundMatch) {
            matchedTargets.push_back(airdropTarget);
        } else {
            unmatchedTargets.push_back(airdropTarget);
        }
    }

    return PipelineResults(imageData, matchedTargets, unmatchedTargets);
}
