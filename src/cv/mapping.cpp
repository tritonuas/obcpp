
#include "cv/mapping.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

void Mapping::loadImages(const std::string& input_path) {
    // Iterate over the directory
    for (const auto& entry : fs::directory_iterator(input_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto ext = entry.path().extension().string();
        // Convert extension to lower case
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
            cv::Mat img = cv::imread(entry.path().string());
            if (!img.empty()) {
                images.push_back(img);
                image_filenames.push_back(entry.path().string());
            } else {
                std::cerr << "Warning: Could not read image "
                          << entry.path().string() << std::endl;
            }
        }
    }

    std::cout << "Images are loaded." << std::endl;
}

void Mapping::mapImages(const std::string& output_path) {
    // Minimum number of images required to stitch
    const int min_img_to_stitch = 2;

    // Check if there are enough images to stitch
    if (static_cast<int>(images.size()) < min_img_to_stitch) {
        std::cerr << "Need at least two images to stitch." << std::endl;
        return;  // or throw, or handle as needed
    }

    // Create a Stitcher object
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::SCANS);

    std::cout << "Stitching images..." << std::endl;
    cv::Mat stitched_image;
    cv::Stitcher::Status status = stitcher->stitch(images, stitched_image);

    if (status == cv::Stitcher::OK) {
        std::cout << "Stitching completed successfully." << std::endl;

        // Get the current date/time for the output filename
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        // Format the date/time as YYYY-MM-DD-HH-MM-SS
        std::stringstream time_ss;
        time_ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d-%H-%M-%S");
        std::string time_str = time_ss.str();

        // Create output directory if it doesn't exist
        if (!fs::exists(output_path)) {
            fs::create_directories(output_path);
        }

        // Construct output path and save the stitched image
        std::string stitched_filename = output_path + "/" + time_str + ".png";
        if (cv::imwrite(stitched_filename, stitched_image)) {
            std::cout << "Stitched image saved to " << stitched_filename << std::endl;
        } else {
            std::cerr << "Failed to save stitched image to " << stitched_filename << std::endl;
        }
    } else {
        std::cerr << "Stitching failed. Error code: " << static_cast<int>(status) << std::endl;
    }
}
