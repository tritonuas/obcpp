#include <iostream>

#include "cv/yolo.hpp"

/**
 * Tests YOLO and some YOLO config settings on an image
 *
 * arg 1 --> YoloModelPath
 * arg 2 --> ImagePath
 * arg 3 --> Confidence Threshold
 * arg 4 --> OutputPath
 */
int main(int argc, char** argv) {
    // Default file paths and parameters
    std::string modelPath = "../models/yolo-wittner-v2.onnx";
    std::string imagePath = "../tests/integration/images/image.png";
    std::string outputPath = "../tests/integration/output/output_yolo.png";
    float confThreshold = 0.35f;

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
        outputPath = argv[4];
    }

    // Load an image using OpenCV
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image at " << imagePath << std::endl;
        return -1;
    }

    // Create the YOLO object and process the image
    YOLO yolo(modelPath, confThreshold, 1024, 1024);
    yolo.processAndSaveImage(image, outputPath);

    return 0;
}
