#include "cv/mapping.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/stitching.hpp"

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
cv::Mat Mapping::readAndResize(const cv::Mat& input, int max_dim) {
    if (input.empty()) return cv::Mat();
    int largest_side = std::max(input.rows, input.cols);
    if (largest_side > max_dim) {
        float scale = static_cast<float>(max_dim) / largest_side;
        cv::Mat resized;
        cv::resize(input, resized, cv::Size(), scale, scale, cv::INTER_AREA);
        return resized;
    }
    return input.clone();
}

std::vector<std::vector<int>> Mapping::chunkListWithOverlap(int num_images, int chunk_size,
                                                            int overlap) {
    if (overlap >= chunk_size) {
        throw std::runtime_error("Overlap must be smaller than chunk_size.");
    }

    std::vector<std::vector<int>> chunks;
    int step = chunk_size - overlap;

    for (int i = 0; i < num_images; i += step) {
        std::vector<int> chunk;
        for (int j = i; j < std::min(i + chunk_size, num_images); ++j) {
            chunk.push_back(j);
        }
        chunks.push_back(chunk);
    }
    return chunks;
}

std::pair<cv::Stitcher::Status, cv::Mat> Mapping::globalStitch(const std::vector<cv::Mat>& images,
                                                               cv::Stitcher::Mode mode,
                                                               int max_dim) {
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(mode);

    // Resize
    std::vector<cv::Mat> resized;
    resized.reserve(images.size());
    for (const auto& im : images) {
        cv::Mat tmp = readAndResize(im, max_dim);
        if (!tmp.empty()) {
            resized.push_back(tmp);
        }
    }

    if (resized.size() < 2) {
        // Not enough images to stitch
        return {cv::Stitcher::ERR_NEED_MORE_IMGS, cv::Mat()};
    }

    cv::Mat stitched;
    cv::Stitcher::Status status;
    try {
        status = stitcher->stitch(resized, stitched);
    } catch (const cv::Exception& e) {
        std::cerr << "[globalStitch] OpenCV exception: " << e.what() << "\n";
        // Return an error status so the caller can skip saving
        return {cv::Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL, cv::Mat()};
    }

    return {status, stitched};
}

std::string Mapping::currentDateTimeStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&tt);

    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");
    return ss.str();
}

// -----------------------------------------------------------------------------
// processChunk()
// -----------------------------------------------------------------------------
void Mapping::processChunk(const std::vector<cv::Mat>& chunk_images, const std::string& run_subdir,
                           cv::Stitcher::Mode mode, int max_dim) {
    if (chunk_images.empty()) return;

    chunk_counter++;
    std::cout << "\nProcessing chunk #" << chunk_counter << " with " << chunk_images.size()
              << " images.\n";

    // Stitch the chunk
    auto [status, stitched] = globalStitch(chunk_images, mode, max_dim);

    // Build an output filename for this chunk
    std::string chunk_filename = run_subdir + "/chunk_" + std::to_string(chunk_counter) + "_" +
                                 currentDateTimeStr() + ".jpg";

    if (status == cv::Stitcher::OK && !stitched.empty()) {
        if (cv::imwrite(chunk_filename, stitched)) {
            std::cout << "Saved chunk result to: " << chunk_filename << "\n";
        } else {
            std::cerr << "Failed to save chunk result to disk. "
                      << "This chunk will be lost.\n";
        }
    } else {
        // If stitching fails, optionally fallback to saving each original chunk image
        // or skip them. Up to you:
        std::cerr << "Chunk stitching failed, status = " << static_cast<int>(status)
                  << ". Not saving chunk.\n";
    }
}

// -----------------------------------------------------------------------------
// firstPass()
// -----------------------------------------------------------------------------
void Mapping::firstPass(const std::string& input_path, const std::string& run_subdir,
                        int chunk_size, int overlap, cv::Stitcher::Mode mode, int max_dim) {
    // 1. Re-scan directory for image filenames
    std::vector<std::string> all_filenames;
    for (const auto& entry : fs::directory_iterator(input_path)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
            all_filenames.push_back(entry.path().string());
        }
    }

    // 2. Sort them so that we process in a consistent order
    std::sort(all_filenames.begin(), all_filenames.end());

    // 3. If we have processed some images before, skip them
    //    i.e., only handle [processed_image_count ... end]
    if (static_cast<int>(all_filenames.size()) <= processed_image_count) {
        std::cout << "No new images found to process.\n";
        return;
    }

    std::vector<std::string> new_files(all_filenames.begin() + processed_image_count,
                                       all_filenames.end());

    // Update the total image_filenames if needed
    image_filenames = all_filenames;  // or keep them updated incrementally

    // 4. Load the new images into memory
    images.clear();
    for (const auto& filename : new_files) {
        cv::Mat img = cv::imread(filename);
        if (!img.empty()) {
            images.push_back(img);
        } else {
            std::cerr << "Warning: Failed to load image: " << filename << "\n";
        }
    }

    std::cout << "Loaded " << images.size() << " new images.\n";

    // 5. Chunk the new images (which are currently in 'images')
    auto chunks = chunkListWithOverlap(static_cast<int>(images.size()), chunk_size, overlap);

    for (auto& chunk_indices : chunks) {
        // Gather the images for this chunk
        std::vector<cv::Mat> chunk_imgs;
        chunk_imgs.reserve(chunk_indices.size());
        for (int idx : chunk_indices) {
            chunk_imgs.push_back(images[idx]);
        }
        // Process (stitch + save)
        processChunk(chunk_imgs, run_subdir, mode, max_dim);
    }

    // 6. Update how many images we've processed in total
    processed_image_count = static_cast<int>(all_filenames.size());

    // 7. Clear the images from memory after done
    images.clear();
}

// -----------------------------------------------------------------------------
// secondPass()
// -----------------------------------------------------------------------------
void Mapping::secondPass(const std::string& run_subdir, cv::Stitcher::Mode mode, int max_dim) {
    // We want to read the chunk images we saved in 'run_subdir'
    // Identify all chunk files: chunk_*.jpg, chunk_*.png, etc.
    std::vector<std::string> chunk_files;
    for (const auto& entry : fs::directory_iterator(run_subdir)) {
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().filename().string();
        // Basic check: does it start with "chunk_" and end with ".jpg"?
        // Or do a better pattern check if needed.
        if (filename.rfind("chunk_", 0) == 0) {
            // starts with "chunk_"
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
                chunk_files.push_back(entry.path().string());
            }
        }
    }

    if (chunk_files.empty()) {
        std::cerr << "No chunk images found in " << run_subdir << ". Nothing to stitch.\n";
        return;
    }

    // Load these chunk images (should be fewer/larger than original set)
    std::vector<cv::Mat> chunk_images;
    chunk_images.reserve(chunk_files.size());
    for (const auto& cfile : chunk_files) {
        cv::Mat cimg = cv::imread(cfile);
        if (!cimg.empty()) {
            chunk_images.push_back(cimg);
        } else {
            std::cerr << "Warning: could not load chunk image: " << cfile << "\n";
        }
    }

    std::cout << "Loaded " << chunk_images.size() << " chunk images. Attempting final stitch...\n";

    // Stitch them all to produce final
    auto [status, final_stitched] = globalStitch(chunk_images, mode, max_dim);

    if (status == cv::Stitcher::OK && !final_stitched.empty()) {
        // Build final filename
        std::string final_name = run_subdir + "/final_" + currentDateTimeStr() + ".jpg";
        if (cv::imwrite(final_name, final_stitched)) {
            std::cout << "Final image saved to: " << final_name << "\n";
        } else {
            std::cerr << "Failed to save final image to " << final_name << "\n";
        }
    } else {
        std::cerr << "Final stitching failed. Status = " << static_cast<int>(status) << "\n";
    }

    // Optionally clear chunk_images from memory
    chunk_images.clear();
}

// -----------------------------------------------------------------------------
// reset()
// -----------------------------------------------------------------------------
void Mapping::reset() {
    images.clear();
    image_filenames.clear();
    processed_image_count = 0;
    chunk_counter = 0;
    std::cout << "Mapping state has been reset.\n";
}
