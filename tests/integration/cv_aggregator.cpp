#include <chrono>
#include <filesystem>
#include <iostream>
#include <loguru.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "cv/aggregator.hpp"
#include "cv/pipeline.hpp"

namespace fs = std::filesystem;

int main() {
    // Define the directory containing the test images
    std::string imagesDirectory = "../tests/integration/aggregator/";
    std::vector<std::string> imagePaths;

    // Iterate over the directory to collect all .jpg, .jpeg, and .png files
    for (const auto& entry : fs::directory_iterator(imagesDirectory)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
                imagePaths.push_back(entry.path().string());
            }
        }
    }

    if (imagePaths.empty()) {
        std::cerr << "No image files found in directory: " << imagesDirectory << std::endl;
        return 1;
    }

    // Construct the pipeline with the desired YOLO model
    PipelineParams params("../models/yolo11x.onnx",
                          "../tests/integration/output/output_aggregator.jpg",
                          false);  // do_preprocess=false

    Pipeline pipeline(params);

    // Build the aggregator by transferring ownership of the pipeline
    CVAggregator aggregator(std::move(pipeline));

    // Create a mock telemetry object (using the same telemetry for simplicity)
    ImageTelemetry mockTelemetry{38.31568, 76.55006, 75, 20, 100, 5, 3};

    // Submit each image from the directory to the aggregator concurrently
    for (const auto& imagePath : imagePaths) {
        cv::Mat image = cv::imread(imagePath);
        if (image.empty()) {
            std::cerr << "Could not open image: " << imagePath << std::endl;
            continue;
        }
        // Create an ImageData instance for each image
        ImageData imageData(image, 0, mockTelemetry);
        aggregator.runPipeline(imageData);
    }

    // Let the worker threads finish
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Retrieve the aggregated results in a thread-safe manner
    auto lockedResults = aggregator.getResults();
    auto resultsPtr = lockedResults.data;
    if (!resultsPtr) {
        std::cerr << "Error: aggregator returned null results?" << std::endl;
        return 1;
    }

    // Count total detections across all runs
    size_t totalDetections = 0;
    for (const auto& run : resultsPtr->runs) {
        totalDetections += run.bboxes.size();
    }

    // Print out bounding box info for each run
    for (size_t runIdx = 0; runIdx < resultsPtr->runs.size(); ++runIdx) {
        const auto& run = resultsPtr->runs[runIdx];
        std::cout << "=== AggregatedRun #" << runIdx << " ===" << std::endl;

        for (size_t detIdx = 0; detIdx < run.bboxes.size(); ++detIdx) {
            const auto& bbox = run.bboxes[detIdx];
            const auto& coord = run.coords[detIdx];

            std::cout << "  Detection #" << detIdx << "  BBox=[" << bbox.x1 << "," << bbox.y1 << ","
                      << bbox.x2 << "," << bbox.y2 << "]"
                      << "  Lat=" << coord.latitude() << "  Lon=" << coord.longitude() << std::endl;
        }
    }

    return 0;
}
