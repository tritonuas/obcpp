#include <iostream>
#include <loguru.hpp>
#include <opencv2/opencv.hpp>

#include "cv/pipeline.hpp"

// Mock telemetry data
const double latitude = 32.990795399999996;
const double longitude = -117.1282463;
const double altitude = 30.108001708984375;
const double airspeed = 7.378872394561768;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

int main(int argc, char** argv) {
    std::string yoloModelPath = "../models/yolo11x.onnx";
    std::string imagePath = "../tests/integration/images/image3.jpg";
    std::string outputPath = "../tests/integration/output/output_pipeline.jpg";

    bool do_preprocess = true;  // Default: perform preprocessing

    if (argc > 1) {
        yoloModelPath = argv[1];
    }
    if (argc > 2) {
        imagePath = argv[2];
    }
    if (argc > 3) {
        outputPath = argv[3];
    }
    // Optional fourth argument to disable preprocessing (e.g., "false" or "0")
    if (argc > 4) {
        std::string preprocessArg = argv[4];
        if (preprocessArg == "false" || preprocessArg == "0") {
            do_preprocess = false;
        }
    }

    // Load test image
    cv::Mat image = cv::imread(imagePath);
    if (!image.data) {
        LOG_F(ERROR, "Failed to open testing image from %s", imagePath.c_str());
        return 1;
    }

    // Prepare telemetry data
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed, yaw, pitch, roll);
    ImageData imageData(image, 0, mockTelemetry);

    // Construct the pipeline with the given parameters.
    PipelineParams params(yoloModelPath, outputPath, do_preprocess);
    Pipeline pipeline(params);

    // Run pipeline
    PipelineResults output = pipeline.run(imageData);

    // Report number of detected targets.
    size_t numTargets = output.targets.size();
    LOG_F(INFO, "Detected %ld targets.", numTargets);

    // Print details for each detected target.
    for (size_t i = 0; i < output.targets.size(); ++i) {
        const auto& t = output.targets[i];
        LOG_F(INFO,
              "Target #%ld: class_id=%d, match_distance=%.2f, "
              "bbox=[%d %d %d %d], lat=%.5f, lon=%.5f",
              i,
              static_cast<int>(t.likely_bottle),  // YOLO class index
              t.match_distance, t.bbox.x1, t.bbox.y1, t.bbox.x2, t.bbox.y2, t.coord.latitude(),
              t.coord.longitude());
    }

    return 0;
}
