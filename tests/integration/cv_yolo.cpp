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

    // NEW: Let YOLO handle drawing/printing
    yolo.drawAndPrintDetections(image, results);

    // Save the output image
    std::string outputFile = "../tests/integration/images/output_yolo.jpg";
    if (!cv::imwrite(outputFile, image)) {
        std::cerr << "Failed to write output image to " << outputFile << std::endl;
        return -1;
    } else {
        std::cout << "Output saved to " << outputFile << std::endl;
    }

    return 0;
}
