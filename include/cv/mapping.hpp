#ifndef INCLUDE_CV_MAPPING_HPP_
#define INCLUDE_CV_MAPPING_HPP_

#include <string>
#include <utility>
#include <vector>

#include "opencv2/opencv.hpp"

// Mapping is responsible for stitching images incrementally in a two-pass approach:
//
//   1) firstPass:
//      - finds new images in 'input_path' that haven't been processed yet
//      - splits them into chunks
//      - stitches each chunk, saves result on disk (not in memory)
//
//   2) secondPass:
//      - scans for all chunk results in the output directory
//      - merges them into one final panorama
//
class Mapping {
 public:
    Mapping() = default;
    ~Mapping() = default;

    // First pass:
    //   - Scans input_path for new images
    //   - Processes them in overlapping chunks
    //   - Saves partial results to run_subdir
    //   - Frees memory for partial results
    void firstPass(const std::string& input_path, const std::string& run_subdir, int chunk_size,
                   int overlap, cv::Stitcher::Mode mode, int max_dim);

    // Second pass:
    //   - Loads all partial chunk files from run_subdir
    //   - Merges them into one final panorama
    //   - Saves final panorama to run_subdir
    void secondPass(const std::string& run_subdir, cv::Stitcher::Mode mode, int max_dim);

    // Optional: reset tracking so you can reuse this object for new sets of images.
    // This clears the counters but does NOT delete your saved chunk images on disk.
    void reset();

 private:
    // Helper: read & optionally downscale an image so its largest dimension <= max_dim
    static cv::Mat readAndResize(const cv::Mat& input, int max_dim);

    // Helper: break a list of indices into overlapping chunks
    static std::vector<std::vector<int>> chunkListWithOverlap(int num_images, int chunk_size,
                                                              int overlap);

    // Helper: performs a single global stitch on a vector of images.
    // Returns (Stitcher::Status, stitched_image).
    static std::pair<cv::Stitcher::Status, cv::Mat> globalStitch(const std::vector<cv::Mat>& images,
                                                                 cv::Stitcher::Mode mode,
                                                                 int max_dim);

    // Helper: generate a timestamp string like "2025-01-24-12-34-56"
    static std::string currentDateTimeStr();

    // Internal: process a single chunk of images, store result on disk,
    //           and free them from memory
    void processChunk(const std::vector<cv::Mat>& chunk_images, const std::string& run_subdir,
                      cv::Stitcher::Mode mode, int max_dim);

 private:
    // All recognized image filenames in the directory (sorted)
    std::vector<std::string> image_filenames;

    // Already-loaded cv::Mat images for the current pass
    std::vector<cv::Mat> images;

    // The number of images we have already processed so far.
    int processed_image_count = 0;

    // For naming the chunk files we produce
    int chunk_counter = 0;
};

#endif  // INCLUDE_CV_MAPPING_HPP_
