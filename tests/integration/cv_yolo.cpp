#include <iostream>

#include "cv/yolo.hpp"

int main(int argc, char** argv) {
    // Default file paths and parameters
    std::string modelPath = "../models/yolo11x.onnx";
    std::string imagePath = "../tests/integration/images/image3.jpg";
    std::string outputFile = "../tests/integration/output/output_yolo.jpg";
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
    if (argc > 4) {
        outputFile = argv[4];
    }

    // Load an image using OpenCV
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image at " << imagePath << std::endl;
        return -1;
    }

    // Create the YOLO object and process the image
    YOLO yolo(modelPath, confThreshold, 640, 640);
    yolo.processAndSaveImage(image, outputFile);

    return 0;
}
