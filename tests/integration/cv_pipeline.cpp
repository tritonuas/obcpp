#include <iostream>
#include <string>
#include <vector>

#include "cv/pipeline.hpp"
#include <loguru.hpp>
#include <opencv2/opencv.hpp>

// Helper to parse comma-separated prompts
static std::vector<std::string> splitCSV(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// Mock telemetry data
const double latitude = 32.990795399999996;
const double longitude = -117.1282463;
const double altitude = 30.108001708984375;
const double airspeed = 7.378872394561768;
const double yaw = 100;
const double pitch = 5;
const double roll = 3;

/**
 * Usage: cv_pipeline <prompts_csv> [image_path]
 * Example: cv_pipeline "person,tent" ../tests/integration/images/test.jpg
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: cv_pipeline <prompts_csv> [image_path]" << std::endl;
        return 1;
    }

    // Hardcoded paths relative to build directory
    std::string encoderPath = "../models/sam3_encoder.onnx";
    std::string decoderPath = "../models/sam3_decoder.onnx";
    std::string tokenizerPath = "../configs/sam3/tokenizer.json";
    std::string imagePath = "../tests/integration/images/tent1.png";
    std::string outputPath = "../tests/integration/output/output_pipeline.jpg";
    bool doPreprocess = false;

    // Parse prompts from command line
    std::vector<std::string> prompts = splitCSV(argv[1]);

    // Optional image path
    if (argc > 2) {
        imagePath = argv[2];
    }

    // Load image
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        LOG_F(ERROR, "Failed to open image: %s", imagePath.c_str());
        return 1;
    }

    // Mock telemetry
    ImageTelemetry mockTelemetry(latitude, longitude, altitude, airspeed, yaw, pitch, roll);
    ImageData imageData(image, 0, mockTelemetry);

    // Create pipeline
    PipelineParams params(encoderPath, decoderPath, tokenizerPath, prompts, outputPath,
                          doPreprocess, 0.30 /*min_confidence*/, 0.20 /*nms_iou*/);
    Pipeline pipeline(params);

    // Run pipeline
    PipelineResults output = pipeline.run(imageData);

    // Report results
    LOG_F(INFO, "Detected %zu targets.", output.targets.size());
    for (size_t i = 0; i < output.targets.size(); ++i) {
        const auto& t = output.targets[i];
        LOG_F(INFO, "Target #%zu: bbox=[%d,%d,%d,%d] lat=%.5f lon=%.5f", i, t.bbox.x1, t.bbox.y1,
              t.bbox.x2, t.bbox.y2, t.coord.latitude(), t.coord.longitude());
    }

    return 0;
}
