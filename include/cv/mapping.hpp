#ifndef INCLUDE_CV_MAPPING_HPP_
#define INCLUDE_CV_MAPPING_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

// Mapping is responsible for mapping the set of images taken from the PyCam to an single mapped
// image.

class Mapping {
 public:
    Mapping() = default;
    ~Mapping() = default;

    // Collect all images from the input path with .jpg/.png/.jpeg
    void loadImages(const std::string& input_path);

    // The mapping function itself
    void mapImages(const std::string& output_path, const int chunk_size = 5,
                   const int chunk_overlap = 2, const int max_dim = 5000);

 private:
    // Read and optionally downscale an image so its largest dimension <= max_dim
    static cv::Mat readAndResize(const cv::Mat& input, int max_dim);

    // Chunk the list of indices into overlapping subsets
    static std::vector<std::vector<int>> chunkListWithOverlap(int num_images, int chunk_size,
                                                              int overlap);

    // Perform a single-pass stitch on a set of already-loaded images (they will be resized here)
    static std::pair<cv::Stitcher::Status, cv::Mat> globalStitch(const std::vector<cv::Mat>& images,
                                                                 cv::Stitcher::Mode mode,
                                                                 int max_dim);

    // Utility to get a timestamp string like "2025-01-24-12-34-56"
    static std::string currentDateTimeStr();

    // Instance variables for the image objects and their respective filenames
    std::vector<cv::Mat> images;
    std::vector<std::string> image_filenames;
};

#endif  // CV_MAPPING_HPP