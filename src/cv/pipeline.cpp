#include "cv/pipeline.hpp"

#include <atomic>

#include "protos/obc.pb.h"
#include "utilities/logging.hpp"

// Pipeline constructor: initialize YOLO detector and the preprocess flag.
Pipeline::Pipeline(const PipelineParams& p)
    : outputPath(p.outputPath),
      do_preprocess(p.do_preprocess) {
    if (p.yoloModelPath.has_value() && !p.yoloModelPath->empty()) {
        yoloDetector = std::make_unique<YOLO>(*p.yoloModelPath, p.detection_threshold, p.inputWidth, p.inputHeight);
        LOG_F(INFO, "YOLO model loaded from path: %s", p.yoloModelPath->c_str());
    } else {
        yoloDetector.reset();
        LOG_F(WARNING, "No CV models are loaded (no YOLO model path provided).");
        LOG_F(WARNING, "CVAGGREGATOR WILL NOT BE WORKING AS INTENDED. USE AT YOUR OWN RISK.");
        LOG_F(WARNING, "Provide a YOLO model path to enable detections.");
    }
}

PipelineResults Pipeline::run(const ImageData& imageData) {
    LOG_F(INFO, "Running pipeline on an image");

    // Preprocess the image if enabled; otherwise, use the original.
    cv::Mat processedImage = imageData.DATA;
    if (do_preprocess) {
        processedImage = preprocessor.cropRight(imageData.DATA);
    }

    // 1) YOLO DETECTION using the (possibly preprocessed) image
    std::vector<Detection> yoloResults = this->yoloDetector->detect(processedImage);

    // If YOLO finds no potential targets, we can return early (still saving out the final image).
    if (yoloResults.empty()) {
        LOG_F(INFO, "No YOLO detections, terminating...");

        if (!outputPath.empty()) {
            static std::atomic<int> file_counter{0};
            int i = file_counter.fetch_add(1);

            // Build a unique filename
            std::string uniqueOutputPath;
            size_t dotPos = outputPath.find_last_of('.');
            size_t sepPos = outputPath.find_last_of("/\\");
            if (dotPos != std::string::npos && (sepPos == std::string::npos || dotPos > sepPos)) {
                uniqueOutputPath = outputPath.substr(0, dotPos) + "_" + std::to_string(i) +
                                   outputPath.substr(dotPos);
            } else {
                uniqueOutputPath = outputPath + "_" + std::to_string(i) + ".jpg";
            }

            if (!cv::imwrite(uniqueOutputPath, processedImage)) {
                LOG_F(ERROR, "Failed to write image to %s", uniqueOutputPath.c_str());
            } else {
                LOG_F(INFO, "Output image saved to %s", uniqueOutputPath.c_str());
            }
        }

        // Return the processed image (with no detections)
        ImageData processedData = imageData;
        processedData.DATA = processedImage;
        return PipelineResults(processedData, {});
    }

    // 2) BUILD DETECTED TARGETS & LOCALIZE
    std::vector<DetectedTarget> detectedTargets;
    detectedTargets.reserve(yoloResults.size());

    for (const auto& det : yoloResults) {
        // Fill bounding box
        Bbox box;
        box.x1 = static_cast<int>(det.x1);
        box.y1 = static_cast<int>(det.y1);
        box.x2 = static_cast<int>(det.x2);
        box.y2 = static_cast<int>(det.y2);

        // If you want to keep cropping code, you can call:
        // cv::Mat cropped = crop(processedImage, box);

        // Localize
        GPSCoord targetPosition;
        if (imageData.TELEMETRY.has_value()) {
            targetPosition = this->gsdLocalizer.localize(imageData.TELEMETRY.value(), box);
        }

        // Populate your DetectedTarget
        DetectedTarget detected;
        detected.bbox = box;
        detected.coord = targetPosition;
        detected.likely_airdrop = static_cast<AirdropType>(det.class_id);
        detected.match_distance = (det.confidence > 0.f) ? (1.0 / det.confidence) : 9999.0;

        detectedTargets.push_back(detected);
    }

    // 3) DRAW DETECTIONS ON THE IMAGE
    //    (this modifies processedImage in-place)
    if (this->yoloDetector) {
        this->yoloDetector->drawAndPrintDetections(processedImage, yoloResults);
    }

    // Save the annotated image if an output path is specified
    if (!outputPath.empty()) {
        static std::atomic<int> file_counter{0};
        int i = file_counter.fetch_add(1);
        std::string uniqueOutputPath;

        size_t dotPos = outputPath.find_last_of('.');
        size_t sepPos = outputPath.find_last_of("/\\");
        if (dotPos != std::string::npos && (sepPos == std::string::npos || dotPos > sepPos)) {
            uniqueOutputPath =
                outputPath.substr(0, dotPos) + "_" + std::to_string(i) + outputPath.substr(dotPos);
        } else {
            uniqueOutputPath = outputPath + "_" + std::to_string(i) + ".jpg";
        }

        if (!cv::imwrite(uniqueOutputPath, processedImage)) {
            LOG_F(ERROR, "Failed to write image to %s", uniqueOutputPath.c_str());
        } else {
            LOG_F(INFO, "Output image saved to %s", uniqueOutputPath.c_str());
        }
    }

    LOG_F(INFO, "Finished Pipeline on an image");

    // Wrap up the final annotated image to return
    ImageData processedData = imageData;
    processedData.DATA = processedImage;

    // Return a PipelineResults that includes:
    //  1) The final big image with bounding boxes drawn
    //  2) The bounding boxes + localized GPS coords
    return PipelineResults(processedData, detectedTargets);
}
