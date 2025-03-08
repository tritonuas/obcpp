#include "cv/pipeline.hpp"

#include <atomic>

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
            // Generate a unique filename using a static atomic counter.
            static std::atomic<int> file_counter{0};
            int i = file_counter.fetch_add(1);
            std::string uniqueOutputPath;
            // Find last dot and separator to check for file extension
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
        ImageData processedData = imageData;
        processedData.DATA = processedImage;
        return PipelineResults(processedData, {});
    }

    // 2) CONSTRUCT DETECTED TARGETS & LOCALIZE
    std::vector<DetectedTarget> detectedTargets;
    detectedTargets.reserve(yoloResults.size());
    for (const auto& det : yoloResults) {
        Bbox box;
        box.x1 = static_cast<int>(det.x1);
        box.y1 = static_cast<int>(det.y1);
        box.x2 = static_cast<int>(det.x2);
        box.y2 = static_cast<int>(det.y2);

        CroppedTarget target;
        target.bbox = box;
        target.croppedImage = crop(processedImage, box);
        target.isMannikin = (det.class_id == 1);

        GPSCoord targetPosition;
        if (imageData.TELEMETRY.has_value()) {
            targetPosition = this->ecefLocalizer.localize(imageData.TELEMETRY.value(), box);
        }
        detectedTargets.push_back(
            DetectedTarget{targetPosition, static_cast<BottleDropIndex>(det.class_id),
                           (det.confidence > 0.f) ? 1.0 / det.confidence : 9999.0, target});
    }

    // Draw detections on the image
    this->yoloDetector->drawAndPrintDetections(processedImage, yoloResults);

    // Save the output image if an output path is provided using a unique name
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
    ImageData processedData = imageData;
    processedData.DATA = processedImage;
    return PipelineResults(processedData, detectedTargets);
}
