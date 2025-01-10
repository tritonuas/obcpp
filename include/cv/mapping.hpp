#ifndef CV_MAPPING_HPP
#define CV_MAPPING_HPP
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

// Mapping is responsible for mapping the set of images taken from the PyCam to an single mapped image.

class Mapping {
   public:
    Mapping() = default;
    ~Mapping() = default;

    // Collect all images from the input path with .jpg/.png/.jpeg
    void loadImages(const std::string& input_path);

    // The mapping function itself
    void mapImages(const std::string& output_path);

   private:
    // Instance variables for the image objects and their respective filenames
    std::vector<cv::Mat> images;
    std::vector<std::string> image_filenames;
};

#endif  // CV_MAPPING_HPP