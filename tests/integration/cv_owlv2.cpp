#include <iostream>

#include "cv/owlv2.hpp"

int main(int argc, char** argv) {
    // Default file paths and parameters
    std::string modelPath = "../models/owlv2.onnx";
    std::string imagePath = "../tests/integration/images/000000000.jpg";
    std::string outputPath = "../tests/integration/output/output_owlv2.jpg";
    std::string tokensPath = "../models/labels_tokenized.json";

    if (argc > 1) {
        modelPath = argv[1];
    }
    if (argc > 2) {
        imagePath = argv[2];
    }
    if (argc > 3) {
        tokensPath = argv[3];
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

    // Create the OWLv2 object and process the image
    OWLv2 owlv2(modelPath, 960, 0.10f, 0.50f, 100, tokensPath);
    owlv2.processAndSaveImage(image, outputPath);

    return 0;
}
