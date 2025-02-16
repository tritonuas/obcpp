#include <iostream>
#include <loguru.hpp>
#include <opencv2/opencv.hpp>

#include "cv/pipeline.hpp"

// Example image path
const std::string imagePath = "../tests/integration/images/000000001.jpg";

// Example YOLO model path
// (Previously named saliencyModelPath, you can rename it as you wish.)
const std::string yoloModelPath = "../models/yolo11x.onnx";

// Mock telemetry data
const double latitude = 32.990795399999996;
const double longitude = -117.1282463;
const double altitude = 30.108001708984375;
const double airspeed = 7.378872394561768;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

int main() {
    // Load test image
    cv::Mat image = cv::imread(imagePath);
    if (!image.data) {
        LOG_F(ERROR, "Failed to open testing image from %s", imagePath.c_str());
        return 1;
    }

    // Prepare telemetry
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed, yaw, pitch, roll);
    ImageData imageData(image, 0, mockTelemetry);

    // Construct pipeline
    // Only YOLO path is used; we ignore all other references.
    PipelineParams params(yoloModelPath /* pass any config you need */);
    Pipeline pipeline(params);

    // Run pipeline
    PipelineResults output = pipeline.run(imageData);

    // Show how many targets YOLO + localization found
    size_t numTargets = output.targets.size();
    LOG_F(INFO, "Detected %ld targets.", numTargets);

    // Print info for each target
    for (size_t i = 0; i < output.targets.size(); ++i) {
        const auto &t = output.targets[i];
        LOG_F(INFO,
              "Target #%ld: class_id=%d, match_distance=%.2f, "
              "bbox=[%d %d %d %d], lat=%.5f, lon=%.5f",
              i,
              static_cast<int>(t.likely_bottle),  // stored YOLO class_id
              t.match_distance, t.crop.bbox.x1, t.crop.bbox.y1, t.crop.bbox.x2, t.crop.bbox.y2,
              t.coord.latitude(), t.coord.longitude());
    }

    return 0;
}
