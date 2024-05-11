#include "cv/pipeline.hpp"
#include "utilities/logging.hpp"

// TODO: eventually we will need to invoke non-default constructors for all the
// modules of the pipeline (to customize model filepath, etc...)
// TODO: I also want to have a way to customize if the model will use
// matching vs segmentation/classification
Pipeline::Pipeline(const PipelineParams& p) :
    // assumes reference images passed to pipeline from not_stolen
        matcher(p.competitionObjectives, p.referenceImages, p.matchingModelPath),
        segmentor(p.segmentationModelPath),
        detector(p.saliencyModelPath) {}

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
    LOG_F(INFO, "Running pipeline on an image");

    VLOG_F(TRACE, "Saliency Start");
    std::vector<CroppedTarget> saliencyResults = this->detector.salience(imageData.DATA);

    // if saliency finds no potential targets, end early with no results
    if (saliencyResults.empty()) {
        LOG_F(INFO, "No saliency results, terminating...");
        return PipelineResults(imageData, {});
    }

    VLOG_F(TRACE, "Saliency cropped %ld potential targets", saliencyResults.size());
    std::vector<DetectedTarget> detectedTargets;
    size_t curr_target_num = 0;
    for (CroppedTarget target : saliencyResults) {
        VLOG_F(TRACE, "Working on target %ld/%ld", ++curr_target_num, saliencyResults.size());
        VLOG_F(TRACE, "Matching Start");
        MatchResult potentialMatch = this->matcher.match(target);

        VLOG_F(TRACE, "Localization Start");
        // TODO: determine what to do if image is missing telemetry metadata
        GPSCoord targetPosition;
        if (imageData.TELEMETRY.has_value()) {
            targetPosition = GPSCoord(this->ecefLocalizer.localize(
                imageData.TELEMETRY.value(), target.bbox));
        }

        VLOG_F(TRACE, "Detected target %ld/%ld at [%f, %f] matched to bottle %d with %f distance.",
            curr_target_num, saliencyResults.size(),
            targetPosition.latitude(), targetPosition.longitude(),
            static_cast<int>(potentialMatch.bottleDropIndex), potentialMatch.distance);
        detectedTargets.push_back(DetectedTarget(targetPosition,
            potentialMatch.bottleDropIndex, potentialMatch.distance));
    }

    LOG_F(INFO, "Finished Pipeline on an image");
    return PipelineResults(imageData, detectedTargets);
}
