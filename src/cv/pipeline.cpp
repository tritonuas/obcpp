#include "cv/pipeline.hpp"

#include "protos/obc.pb.h"
#include "utilities/logging.hpp"

// Pipeline constructor: initialize YOLO detector and the preprocess flag.
Pipeline::Pipeline(const PipelineParams& p)
    : yoloDetector(std::make_unique<YOLO>(p.yoloModelPath, 0.05f, 640, 640)),
      outputPath(p.outputPath),
      do_preprocess(p.do_preprocess) {}

PipelineResults Pipeline::run(const ImageData& imageData) {
    LOG_F(INFO, "Running pipeline on an image");

    // Preprocess the image if enabled; otherwise, use the original image.
    cv::Mat processedImage = imageData.DATA;
    if (do_preprocess) {
        processedImage = preprocessor.cropRight(imageData.DATA);
    }

    // 1) YOLO DETECTION using the (possibly preprocessed) image
    std::vector<Detection> yoloResults = this->yoloDetector->detect(processedImage);

    // If YOLO finds no potential targets, end early.
    if (yoloResults.empty()) {
        LOG_F(INFO, "No YOLO detections, terminating...");
        if (!outputPath.empty()) {
            // Save the preprocessed image (without detections) if an output path is provided.
            if (!cv::imwrite(outputPath, processedImage)) {
                LOG_F(ERROR, "Failed to write image to %s", outputPath.c_str());
            } else {
                LOG_F(INFO, "Output image saved to %s", outputPath.c_str());
            }
        }
        ImageData processedData = imageData;
        processedData.DATA = processedImage;
        return PipelineResults(processedData, {});
    }

    // 2) CONSTRUCT DETECTED TARGETS & LOCALIZE
    std::vector<DetectedTarget> detectedTargets;
    detectedTargets.reserve(yoloResults.size());

    for (const auto& det : yoloResults) {
        // Convert YOLO coordinates to the Bbox struct.
        Bbox box;
        box.x1 = static_cast<int>(det.x1);
        box.y1 = static_cast<int>(det.y1);
        box.x2 = static_cast<int>(det.x2);
        box.y2 = static_cast<int>(det.y2);

        // Build a CroppedTarget: crop using the (preprocessed) image.
        CroppedTarget target;
        target.bbox = box;
        target.croppedImage = crop(processedImage, box);
        // If class_id == 1 then it is a mannikin (depends on model labeling).
        target.isMannikin = (det.class_id == 1);

        // 3) LOCALIZATION (unchanged)
        GPSCoord targetPosition;
        if (imageData.TELEMETRY.has_value()) {
            targetPosition = this->ecefLocalizer.localize(imageData.TELEMETRY.value(), box);
        }

        // 4) Push into final vector. For "likely_bottle" and "match_distance",
        //    we store the class_id and use 1/confidence (or a dummy value) respectively.
        detectedTargets.push_back(
            DetectedTarget{targetPosition, static_cast<BottleDropIndex>(det.class_id),
                           (det.confidence > 0.f) ? 1.0 / det.confidence : 9999.0, target});
    }

    // Draw detections on the image
    this->yoloDetector->drawAndPrintDetections(processedImage, yoloResults);

    // Save the output image if an output path is provided.
    if (!outputPath.empty()) {
        if (!cv::imwrite(outputPath, processedImage)) {
            LOG_F(ERROR, "Failed to write image to %s", outputPath.c_str());
        } else {
            LOG_F(INFO, "Output image saved to %s", outputPath.c_str());
        }
    }

    LOG_F(INFO, "Finished Pipeline on an image");
    ImageData processedData = imageData;
    processedData.DATA = processedImage;
    return PipelineResults(processedData, detectedTargets);
}
