#include "cv/pipeline.hpp"

#include "protos/obc.pb.h"
#include "utilities/logging.hpp"

// Pipeline constructor
Pipeline::Pipeline(const PipelineParams& p)
    : yoloDetector(std::make_unique<YOLO>(p.yoloModelPath, 0.05f, 640, 640)) {}
PipelineResults Pipeline::run(const ImageData& imageData) {
    LOG_F(INFO, "Running pipeline on an image");

    // 1) YOLO DETECTION
    // -------------------------------------------------------------------------
    std::vector<Detection> yoloResults = this->yoloDetector->detect(imageData.DATA);

    // If YOLO finds no potential targets, end early
    if (yoloResults.empty()) {
        LOG_F(INFO, "No YOLO detections, terminating...");
        return PipelineResults(imageData, {});
    }

    // 2) CONSTRUCT DETECTED TARGETS & LOCALIZE
    // -------------------------------------------------------------------------
    std::vector<DetectedTarget> detectedTargets;
    detectedTargets.reserve(yoloResults.size());

    for (const auto& det : yoloResults) {
        // Convert YOLO coords to your Bbox struct
        Bbox box;
        box.x1 = static_cast<int>(det.x1);
        box.y1 = static_cast<int>(det.y1);
        box.x2 = static_cast<int>(det.x2);
        box.y2 = static_cast<int>(det.y2);

        // Build a CroppedTarget (if you still want the cropped image and isMannikin info)
        CroppedTarget target;
        target.bbox = box;
        target.croppedImage = crop(imageData.DATA, box);
        // If class_id == 1 means mannikin (depends on your model’s labeling)
        target.isMannikin = (det.class_id == 1);

        // 3) LOCALIZATION (unchanged)
        // ---------------------------------------------------------------------
        GPSCoord targetPosition;
        if (imageData.TELEMETRY.has_value()) {
            // e.g. use ECEFLocalization
            targetPosition = this->ecefLocalizer.localize(imageData.TELEMETRY.value(), box);
        }

        // 4) Push into final vector
        //    For "likely_bottle" and "match_distance", you can store YOLO info,
        //    or just dummy values. Below we store class_id -> BottleDropIndex,
        //    and 1/confidence as a pseudo "distance".
        // ---------------------------------------------------------------------
        detectedTargets.push_back(
            DetectedTarget{targetPosition, static_cast<BottleDropIndex>(det.class_id),
                           (det.confidence > 0.f) ? 1.0 / det.confidence : 9999.0, target});
    }

    LOG_F(INFO, "Finished Pipeline on an image");
    return PipelineResults(imageData, detectedTargets);
}
