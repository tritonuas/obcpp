#include <iostream>

#include "cv/yolo.hpp"

int main(int argc, char** argv) {
    // Simple argument check
    std::string modelPath = "../models/yolo11x.onnx";
    std::string imagePath = "../tests/integration/images/image3.jpg";
    float confThreshold = 0.05f;

    if (argc > 1) {
        modelPath = argv[1];
    }
    if (argc > 2) {
        imagePath = argv[2];
    }
    if (argc > 3) {
        confThreshold = std::stof(argv[3]);
    }

    // Load an image using OpenCV
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image at " << imagePath << std::endl;
        return -1;
    }

    // Create the YOLO object
    YOLO yolo(modelPath, confThreshold, 640, 640);

    // Perform detection
    std::vector<Detection> results = yolo.detect(image);

    // Print detections and draw bounding boxes
    for (const auto& det : results) {
        std::cout << "Detected class: " << det.class_id << " conf: " << det.confidence << " box: ["
                  << det.x1 << ", " << det.y1 << ", " << det.x2 << ", " << det.y2 << "]"
                  << std::endl;

        // Draw the bounding box
        cv::rectangle(image, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1)),
                      cv::Point(static_cast<int>(det.x2), static_cast<int>(det.y2)),
                      cv::Scalar(0, 255, 0),  // BGR color (green)
                      2                       // thickness
        );

        // Optionally, draw the class ID and confidence near the top-left corner of the box
        std::string label = "ID:" + std::to_string(det.class_id) +
                            " conf:" + std::to_string(det.confidence).substr(0, 4);
        cv::putText(image, label, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1) - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0),  // matching color
                    1);
    }

    // Save the output image
    std::string outputFile = "../test/integration/images/output_yolo.jpg";
    if (!cv::imwrite(outputFile, image)) {
        std::cerr << "Failed to write output image to " << outputFile << std::endl;
        return -1;
    } else {
        std::cout << "Output saved to " << outputFile << std::endl;
    }

    return 0;
}
