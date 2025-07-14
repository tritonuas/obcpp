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
//      - frees memory after processing
//      - **New:** Saves partial results in a new timestamped subfolder within the given run_subdir.
//   2) secondPass:
//      - scans for all chunk files in the timestamped run folder
//      - merges them into one final panorama
//      - saves final panorama to the same folder
//
class Mapping {
 public:
    Mapping() = default;
    ~Mapping() = default;

    // First pass:
    //   - Scans input_path for new images
    //   - Processes them in overlapping chunks
    //   - Saves partial results to a new timestamped folder inside run_subdir
    //   - Frees memory for partial results
    // Added parameter 'preprocess' to determine if images should be pre-processed.
    void firstPass(const std::string& input_path, const std::string& run_subdir, int chunk_size,
                   int overlap, cv::Stitcher::Mode mode, int max_dim, bool preprocess = true);

    // Second pass:
    //   - Loads all partial chunk files from the current timestamped run folder
    //   - Merges them into one final panorama
    //   - Saves final panorama to the same folder
    // Added parameter 'preprocess' to determine if images should be pre-processed.
    void secondPass(const std::string& run_subdir, cv::Stitcher::Mode mode, int max_dim,
                    bool preprocess = true);

    // Optional: reset tracking so you can reuse this object for new sets of images.
    // This clears the counters but does NOT delete your saved chunk images on disk.
    void reset();

    // Direct stitch: loads all images from input_path and stitches them into a single panorama
    // Saves the result to output_path. This bypasses the two-pass chunking system.
    void directStitch(const std::string& input_path, const std::string& output_path,
                      cv::Stitcher::Mode mode, int max_dim, bool preprocess = true);

 private:
    // Helper: read & optionally downscale an image so its largest dimension <= max_dim
    static cv::Mat readAndResize(const cv::Mat& input, int max_dim);

    // Helper: break a list of indices into overlapping chunks
    static std::vector<std::vector<int>> chunkListWithOverlap(int num_images, int chunk_size,
                                                              int overlap);

    // Helper: performs a single global stitch on a vector of images.
    // Returns (Stitcher::Status, stitched_image).
    // Added parameter 'preprocess' to determine if images should be pre-processed.
    static std::pair<cv::Stitcher::Status, cv::Mat> globalStitch(const std::vector<cv::Mat>& images,
                                                                 cv::Stitcher::Mode mode,
                                                                 int max_dim, bool preprocess);

    // Helper: generate a timestamp string like "2025-01-24-12-34-56"
    static std::string currentDateTimeStr();

    // Internal: process a single chunk of images, store result on disk,
    //           and free them from memory.
    // Added parameter 'preprocess' to determine if images should be pre-processed.
    void processChunk(const std::vector<cv::Mat>& chunk_images, const std::string& run_subdir,
                      cv::Stitcher::Mode mode, int max_dim, bool preprocess);

 private:
    // All recognized image filenames in the directory (sorted)
    std::vector<std::string> image_filenames;

    // Already-loaded cv::Mat images for the current pass
    std::vector<cv::Mat> images;

    // The number of images we have already processed so far.
    int processed_image_count = 0;

    // For naming the chunk files we produce
    int chunk_counter = 0;

    // New: the timestamped folder (inside the given run_subdir) for this run.
    // This ensures that subsequent calls (e.g. secondPass) use only the current run's files.
    std::string current_run_folder;
};

#endif  // INCLUDE_CV_MAPPING_HPP_
