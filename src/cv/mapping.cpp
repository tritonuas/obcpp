#include "cv/mapping.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---------------------
// Helper functions
// ---------------------

// Read and optionally downscale an image so its largest dimension <= max_dim
static cv::Mat readAndResize(const cv::Mat& input, int max_dim) {
    if (input.empty()) {
        return cv::Mat();  // return empty on failure
    }
    int h = input.rows;
    int w = input.cols;
    int largest_side = std::max(h, w);
    if (largest_side > max_dim) {
        // Compute the scale factor and downscale
        float scale = static_cast<float>(max_dim) / static_cast<float>(largest_side);
        cv::Mat resized;
        cv::resize(input, resized, cv::Size(), scale, scale, cv::INTER_AREA);
        return resized;
    }
    // Otherwise, no resizing needed
    return input.clone();
}

// Chunk the list of indices into overlapping subsets
static std::vector<std::vector<int>> chunkListWithOverlap(int num_images, int chunk_size, int overlap) {
    if (overlap >= chunk_size) {
        throw std::runtime_error("Overlap must be smaller than chunk_size.");
    }
    std::vector<std::vector<int>> chunks;
    int step = chunk_size - overlap;
    for (int i = 0; i < num_images; i += step) {
        int end = std::min(i + chunk_size, num_images);
        std::vector<int> chunk;
        for (int j = i; j < end; ++j) {
            chunk.push_back(j);
        }
        chunks.push_back(chunk);
    }
    return chunks;
}

// Perform a single-pass stitch on a set of already-loaded images (they will be resized here)
static std::pair<cv::Stitcher::Status, cv::Mat> globalStitch(const std::vector<cv::Mat>& images,
                                                             cv::Stitcher::Mode mode,
                                                             int max_dim) {
    // Prepare the stitcher
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(mode);

    // Resize on-the-fly, collect into a new vector
    std::vector<cv::Mat> resized;
    resized.reserve(images.size());
    for (const auto& im : images) {
        cv::Mat tmp = readAndResize(im, max_dim);
        if (tmp.empty()) {
            // Could not read or something went wrong
            return {cv::Stitcher::ERR_NEED_MORE_IMGS, cv::Mat()};
        }
        resized.push_back(tmp);
    }

    // Run stitching
    cv::Mat stitched;
    cv::Stitcher::Status status = stitcher->stitch(resized, stitched);
    return {status, stitched};
}

// Utility to get a timestamp string like "2025-01-24-12-34-56"
static std::string currentDateTimeStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &tt);
#else
    local_tm = *std::localtime(&tt);
#endif
    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");
    return ss.str();
}

// ---------------------
// Mapping class methods
// ---------------------

void Mapping::loadImages(const std::string& input_path) {
    // Clear any existing data
    images.clear();
    image_filenames.clear();

    // Gather all valid image file paths (jpg/jpeg/png)
    for (const auto& entry : fs::directory_iterator(input_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto ext = entry.path().extension().string();
        // to lower
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
            image_filenames.push_back(entry.path().string());
        }
    }

    if (image_filenames.empty()) {
        std::cout << "No images found in " << input_path << ".\n";
        return;
    }

    // Sort filenames for consistent order
    std::sort(image_filenames.begin(), image_filenames.end());

    // Load each image into memory
    for (const auto& filename : image_filenames) {
        cv::Mat img = cv::imread(filename);
        if (!img.empty()) {
            images.push_back(img);
        } else {
            std::cerr << "[WARNING] Could not read image: " << filename << std::endl;
        }
    }

    std::cout << "Images are loaded. Count: " << images.size() << std::endl;
}

void Mapping::mapImages(const std::string& output_path) {
    // If no images, nothing to do
    if (images.empty()) {
        std::cerr << "No images available to stitch. Exiting.\n";
        return;
    }

    // ----------------------------------
    // Configuration (mirroring Python)
    // ----------------------------------
    const int chunk_size = 5;
    const int chunk_overlap = 2;
    const cv::Stitcher::Mode stitcher_mode = cv::Stitcher::SCANS;
    const int max_dim = 5000;  // resizing limit

    // Create output directory if not exist
    if (!fs::exists(output_path)) {
        fs::create_directories(output_path);
    }

    // Split image indices into overlapping chunks
    std::vector<std::vector<int>> chunked_indices =
        chunkListWithOverlap(static_cast<int>(images.size()), chunk_size, chunk_overlap);

    // We'll store partial results (stitched chunks or original chunk images) here
    std::vector<cv::Mat> partial_results;
    partial_results.reserve(chunked_indices.size());  // Just a guess; might be bigger if fails.

    int chunk_index = 0;
    for (const auto& chunk : chunked_indices) {
        chunk_index++;
        std::cout << "\n[INFO] Stitching chunk #" << chunk_index
                  << " with " << chunk.size() << " images..." << std::endl;

        // Gather cv::Mat for this chunk
        std::vector<cv::Mat> chunk_images;
        chunk_images.reserve(chunk.size());
        for (int idx : chunk) {
            chunk_images.push_back(images[idx]);
        }

        // Attempt to stitch this chunk
        auto [status, stitched] = globalStitch(chunk_images, stitcher_mode, max_dim);

        if (status == cv::Stitcher::OK && !stitched.empty()) {
            // Save partial stitched result
            std::string ts = currentDateTimeStr();
            std::string out_path =
                output_path + "/0_chunk_" + std::to_string(chunk_index) + "_" + ts + ".jpg";
            if (cv::imwrite(out_path, stitched)) {
                std::cout << "[INFO] Chunk #" << chunk_index
                          << " stitched OK -> " << out_path << std::endl;
            } else {
                std::cerr << "[WARNING] Failed to save chunk result to " << out_path << std::endl;
            }

            // Keep this partial panorama in the next pass
            partial_results.push_back(stitched);
        } else {
            std::cout << "[WARNING] Stitch failed for chunk #" << chunk_index
                      << ". Status code: " << static_cast<int>(status) << std::endl;
            // Retain original chunk images for final pass
            std::cout << "[INFO] Retaining original images for final pass." << std::endl;
            // i.e., store them individually in partial_results
            for (auto& cimg : chunk_images) {
                partial_results.push_back(cimg);
            }
        }
    }

    if (partial_results.empty()) {
        std::cerr << "[ERROR] All chunk stitches failed and no original images retained. "
                     "Nothing to merge.\n";
        return;
    }

    // --------------------------
    // Second pass: merge all partial results
    // --------------------------
    std::cout << "\n[INFO] Merging all partial results in one pass..." << std::endl;
    auto [final_status, final_stitched] = globalStitch(partial_results, stitcher_mode, max_dim);
    if (final_status == cv::Stitcher::OK && !final_stitched.empty()) {
        std::string ts = currentDateTimeStr();
        std::string final_out_path = output_path + "/final_merged_" + ts + ".jpg";
        if (cv::imwrite(final_out_path, final_stitched)) {
            std::cout << "[INFO] Final merged panorama -> " << final_out_path << std::endl;
        } else {
            std::cerr << "[WARNING] Failed to save final merged panorama.\n";
        }
    } else {
        std::cerr << "[WARNING] Final merge failed. Status code: "
                  << static_cast<int>(final_status) << std::endl;
        std::cerr << "Partial results remain in memory. You may consider saving them individually.\n";
    }

    std::cout << "\n[INFO] Done.\n";
}
