#include <iostream>
#include <opencv2/opencv.hpp>

#include "cv/preprocess.hpp"

int main(int argc, char* argv[]) {
    // Set default file paths.
    std::string input_image_path = "../tests/integration/images/output_0819.png";
    std::string output_image_path = "../tests/integration/output/preprocess_output.png";

    // Allow file paths to be overridden by command-line arguments.
    if (argc > 1) {
        input_image_path = argv[1];
    }
    if (argc > 2) {
        output_image_path = argv[2];
    }

    // Load the image from disk.
    cv::Mat image = cv::imread(input_image_path);
    if (image.empty()) {
        std::cerr << "Error: Failed to load image: " << input_image_path << std::endl;
        return 1;
    }

    // Instantiate the Preprocess object and crop the image.
    Preprocess preprocess;
    cv::Mat cropped_image = preprocess.cropRight(image);

    // Save the cropped image so you can inspect it manually.
    if (!cv::imwrite(output_image_path, cropped_image)) {
        std::cerr << "Error: Failed to write image to: " << output_image_path << std::endl;
        return 1;
    }

    std::cout << "Image processed successfully. Cropped image saved to: " << output_image_path
              << std::endl;
    return 0;
}
