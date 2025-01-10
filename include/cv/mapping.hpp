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
    // Probably will only need the input and output paths
    void mapImages(const std::string& input_path, const std::string& output_path);

    // Add instance variables
};

#endif  // CV_MAPPING_HPP