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
                          "../tests/integration/output/output_aggregator.jpg", false);

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

    // Optional: To simulate a heavier load, you could submit each image multiple times
    // for (int i = 0; i < 3; i++) {
    //     for (const auto& imagePath : imagePaths) {
    //         cv::Mat image = cv::imread(imagePath);
    //         if (image.empty()) {
    //             std::cerr << "Could not open image: " << imagePath << std::endl;
    //             continue;
    //         }
    //         ImageData imageData(image, 0, mockTelemetry);
    //         aggregator.runPipeline(imageData);
    //     }
    // }

    // Allow time for all worker threads to process the images
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Retrieve the aggregated results in a thread-safe manner
    auto lockedResults = aggregator.getResults();
    auto resultsPtr = lockedResults.data;
    if (!resultsPtr) {
        std::cerr << "Error: aggregator returned null results?" << std::endl;
        return 1;
    }

    // Print out the total number of detections and details for each detection
    std::cout << "Total detections after aggregator finished: "
              << resultsPtr->detected_targets.size() << std::endl;

    for (size_t i = 0; i < resultsPtr->detected_targets.size(); ++i) {
        const auto& det = resultsPtr->detected_targets[i];
        std::cout << "Detection #" << i << " bottle=" << static_cast<int>(det.likely_bottle)
                  << " dist=" << det.match_distance << " lat=" << det.coord.latitude()
                  << " lon=" << det.coord.longitude() << std::endl;
    }

    return 0;
}
