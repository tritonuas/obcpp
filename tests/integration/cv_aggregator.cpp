#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cv/aggregator.hpp"
#include "cv/pipeline.hpp"
#include <loguru.hpp>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

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

int main(int argc, char** argv) {
    // Usage: cv_aggregator <prompts_csv>
    if (argc < 2) {
        std::cerr << "Usage: cv_aggregator <prompts_csv>" << std::endl;
        return 1;
    }

    // Hardcoded paths relative to build directory
    std::string encoderPath = "../models/sam3_encoder.onnx";
    std::string decoderPath = "../models/sam3_decoder.onnx";
    std::string tokenizerPath = "../configs/sam3/tokenizer.json";
    std::string inputDir = "../tests/integration/images/";
    std::string outputPath = "../tests/integration/output/output_aggregator.jpg";
    bool doPreprocess = false;

    // Parse prompts from command line argument
    std::vector<std::string> prompts = splitCSV(argv[1]);

    // Construct the PipelineParams
    PipelineParams params(encoderPath, decoderPath, tokenizerPath, prompts, outputPath,
                          doPreprocess, 0.30 /*min_confidence*/, 0.20 /*nms_iou*/);

    // 1. Create the pipeline on the stack (no shared_ptr)
    Pipeline pipeline(params);

    // 2. Build the aggregator by transferring ownership using std::move
    CVAggregator aggregator(std::move(pipeline));

    // Mock telemetry
    ImageTelemetry mockTelemetry{38.31568, 76.55006, 75, 20, 100, 5, 3};

    std::vector<std::string> imagePaths;
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
                imagePaths.push_back(entry.path().string());
            }
        }
    }

    if (imagePaths.empty()) {
        std::cerr << "No image files found in directory: " << inputDir << std::endl;
        return 1;
    }

    // Submit images to aggregator
    for (const auto& imagePath : imagePaths) {
        cv::Mat image = cv::imread(imagePath);
        if (image.empty()) {
            std::cerr << "Could not open image: " << imagePath << std::endl;
            continue;
        }
        ImageData imageData(image, 0, mockTelemetry);
        aggregator.runPipeline(imageData);
    }

    // Allow workers to finish
    std::this_thread::sleep_for(std::chrono::seconds(120));

    // Retrieve results
    auto lockedResults = aggregator.getResults();
    auto resultsPtr = lockedResults.data;
    if (!resultsPtr) {
        std::cerr << "Error: aggregator returned null results?" << std::endl;
        return 1;
    }

    size_t totalDetections = 0;
    for (const auto& run : resultsPtr->runs) {
        totalDetections += run.bboxes.size();
    }

    std::cout << "\nPrompts: ";
    for (size_t i = 0; i < prompts.size(); ++i) {
        std::cout << prompts[i] << (i + 1 < prompts.size() ? "," : "");
    }
    std::cout << " | Total detections: " << totalDetections << std::endl;

    for (size_t runIdx = 0; runIdx < resultsPtr->runs.size(); ++runIdx) {
        const auto& run = resultsPtr->runs[runIdx];
        std::cout << "--- AggregatedRun #" << runIdx << " ---" << std::endl;

        for (size_t detIdx = 0; detIdx < run.bboxes.size(); ++detIdx) {
            const auto& bbox = run.bboxes[detIdx];
            const auto& coord = run.coords[detIdx];

            std::cout << "  Det #" << detIdx << "  BBox=[" << bbox.x1 << "," << bbox.y1 << ","
                      << bbox.x2 << "," << bbox.y2 << "]"
                      << "  Lat=" << coord.latitude() << "  Lon=" << coord.longitude() << std::endl;
        }
    }

    return 0;
}