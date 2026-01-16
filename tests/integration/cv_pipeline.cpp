#include <iostream>
#include <sstream>
#include <vector>

#include "cv/pipeline.hpp"
#include <loguru.hpp>
#include <opencv2/opencv.hpp>

// Mock telemetry data
const double latitude = 32.990795399999996;
const double longitude = -117.1282463;
const double altitude = 30.108001708984375;
const double airspeed = 7.378872394561768;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

/**
 * Runs a single instance of the pipeline
 *
 * arg 1 --> ModelPath
 * arg 2 --> TokenizerPath
 * arg 3 --> ImagePath
 * arg 4 --> OutputPath
 * arg 5 --> enable preprocessing ("true" or "false") - default true
 * arg 6 --> prompt text (optional, default: "person")
 * arg 7 --> min_confidence (optional, default: 0.08)
 * arg 8 --> nms_iou (optional, default: 0.5)
 */
int main(int argc, char** argv) {
    std::string modelPath = "../models/sam3_detection.onnx";
    std::string tokenizerPath = "../configs/cv/tokenizer.json";
    std::string imagePath = "../tests/integration/images/image.png";
    std::string outputPath = "../tests/integration/output/output_pipeline.png";

    bool do_preprocess = true;  // Default: perform preprocessing

    if (argc > 1) {
        modelPath = argv[1];
    }
    if (argc > 2) {
        tokenizerPath = argv[2];
    }
    if (argc > 3) {
        imagePath = argv[3];
    }
    if (argc > 4) {
        outputPath = argv[4];
    }
    // Optional fifth argument to disable preprocessing (e.g., "false" or "0")
    if (argc > 5) {
        std::string preprocessArg = argv[5];
        if (preprocessArg == "false" || preprocessArg == "0") {
            do_preprocess = false;
        }
    }
    // Optional sixth argument for prompt(s); supports comma-separated list
    std::vector<std::string> prompts = {"person"};
    if (argc > 6) {
        std::string promptArg = argv[6];
        prompts.clear();
        std::istringstream ss(promptArg);
        std::string token;
        while (std::getline(ss, token, ',')) {
            if (!token.empty()) prompts.push_back(token);
        }
        if (prompts.empty()) {
            prompts.push_back(promptArg);
        }
    }

    double min_confidence = 0.20;
    double nms_iou = 0.2;
    if (argc > 7) {
        min_confidence = std::stod(argv[7]);
    }
    if (argc > 8) {
        nms_iou = std::stod(argv[8]);
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
    PipelineParams params(modelPath, tokenizerPath, prompts, outputPath, do_preprocess,
                          min_confidence, nms_iou);

    // Create pipeline
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
              static_cast<int>(t.likely_airdrop),  // SAM3 class index
              t.match_distance, t.bbox.x1, t.bbox.y1, t.bbox.x2, t.bbox.y2, t.coord.latitude(),
              t.coord.longitude());
    }

    return 0;
}
