#include "cv/pipeline.hpp"

// TODO: eventually we will need to invoke non-default constructors for all the
// modules of the pipeline (to customize model filepath, etc...)
// TODO: I also want to have a way to customize if the model will use
// matching vs segmentation/classification
Pipeline::Pipeline(const PipelineParams& p) :
    // assumes reference images passed to pipeline from not_stolen
        matcher(p.competitionObjectives, p.referenceImages, p.matchingModelPath),
        segmentor(p.segmentationModelPath) {}

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
    std::vector<CroppedTarget> saliencyResults = this->detector.salience(imageData.getData());

    // if saliency finds no potential targets, end early with no results
    if (saliencyResults.empty()) {
        return PipelineResults(imageData, {});
    }

    std::vector<DetectedTarget> detectedTargets;
    for (CroppedTarget target : saliencyResults) {
        MatchResult potentialMatch = this->matcher.match(target);

        GPSCoord targetPosition(this->ecefLocalizer.localize(
            imageData.getTelemetry(), target.bbox));

        detectedTargets.push_back(DetectedTarget(targetPosition, 
            potentialMatch.bottleDropIndex, potentialMatch.distance));
    }

    return PipelineResults(imageData, detectedTargets);
}
