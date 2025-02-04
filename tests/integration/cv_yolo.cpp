#include <iostream>

#include "cv/yolo.hpp"

int main(int argc, char** argv) {
    // Simple argument check
    std::string modelPath = "../models/yolo11n.onnx";
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

    // Print detections
    // Note: If your model's class labels are known, you can map class_id to actual string labels
    // For demonstration, we just print the numeric class_id
    for (const auto& det : results) {
        std::cout << "Detected class: " << det.class_id << " conf: " << det.confidence << " box: ["
                  << det.x1 << ", " << det.y1 << ", " << det.x2 << ", " << det.y2 << "]"
                  << std::endl;
    }

    return 0;
}
