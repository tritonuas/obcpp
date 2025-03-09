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

#include "cv/preprocess.hpp"  // include the Preprocess header
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

// -----------------------------------------------------------------------------
// Updated: globalStitch() now simply clones its images because they have already been pre-processed
// -----------------------------------------------------------------------------
std::pair<cv::Stitcher::Status, cv::Mat> Mapping::globalStitch(const std::vector<cv::Mat>& images,
                                                               cv::Stitcher::Mode mode, int max_dim,
                                                               bool /*preprocess*/) {
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(mode);

    std::vector<cv::Mat> processed;
    processed.reserve(images.size());
    // Simply clone images – they are assumed to be preprocessed already.
    for (const auto& im : images) {
        processed.push_back(im.clone());
    }

    if (processed.size() < 2) {
        // Not enough images to stitch
        return {cv::Stitcher::ERR_NEED_MORE_IMGS, cv::Mat()};
    }

    cv::Mat stitched;
    cv::Stitcher::Status status;
    try {
        status = stitcher->stitch(processed, stitched);
    } catch (const cv::Exception& e) {
        std::cerr << "[globalStitch] OpenCV exception: " << e.what() << "\n";
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
                           cv::Stitcher::Mode mode, int max_dim, bool preprocess) {
    if (chunk_images.empty()) return;

    chunk_counter++;
    std::cout << "\nProcessing chunk #" << chunk_counter << " with " << chunk_images.size()
              << " images.\n";

    // Stitch the chunk using the (now ignored) preprocess flag
    auto [status, stitched] = globalStitch(chunk_images, mode, max_dim, preprocess);

    // Build an output filename for this chunk
    std::string chunk_filename = run_subdir + "/chunk_" + std::to_string(chunk_counter) + "_" +
                                 currentDateTimeStr() + ".jpg";

    if (status == cv::Stitcher::OK && !stitched.empty()) {
        if (cv::imwrite(chunk_filename, stitched)) {
            std::cout << "Saved chunk result to: " << chunk_filename << "\n";
        } else {
            std::cerr << "Failed to save chunk result to disk. This chunk will be lost.\n";
        }
    } else {
        std::cerr << "Chunk stitching failed, status = " << static_cast<int>(status)
                  << ". Not saving chunk.\n";
    }
}

// -----------------------------------------------------------------------------
// firstPass()
// -----------------------------------------------------------------------------
void Mapping::firstPass(const std::string& input_path, const std::string& run_subdir,
                        int chunk_size, int overlap, cv::Stitcher::Mode mode, int max_dim,
                        bool preprocess) {
    // Create a run folder if not already done
    if (current_run_folder.empty()) {
        current_run_folder = run_subdir + "/" + currentDateTimeStr();
        try {
            fs::create_directories(current_run_folder);
            std::cout << "Created run folder: " << current_run_folder << "\n";
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating run folder: " << e.what() << "\n";
            throw;
        }
    }

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

    // 2. Sort them for consistent processing
    std::sort(all_filenames.begin(), all_filenames.end());

    // 3. Skip already processed images
    if (static_cast<int>(all_filenames.size()) <= processed_image_count) {
        std::cout << "No new images found to process.\n";
        return;
    }

    std::vector<std::string> new_files(all_filenames.begin() + processed_image_count,
                                       all_filenames.end());
    image_filenames = all_filenames;

    // 4. Load and preprocess new images
    images.clear();
    Preprocess preprocessor;  // instantiate the Preprocess object
    for (const auto& filename : new_files) {
        cv::Mat img = cv::imread(filename);
        if (!img.empty()) {
            if (preprocess) {
                // Apply custom preprocessing before further processing
                img = preprocessor.cropRight(img);
                img = readAndResize(img, max_dim);
            }
            images.push_back(img);
        } else {
            std::cerr << "Warning: Failed to load image: " << filename << "\n";
        }
    }
    std::cout << "Loaded and preprocessed " << images.size() << " new images.\n";

    // 5. Chunk the preprocessed images
    auto chunks = chunkListWithOverlap(static_cast<int>(images.size()), chunk_size, overlap);
    for (auto& chunk_indices : chunks) {
        std::vector<cv::Mat> chunk_imgs;
        chunk_imgs.reserve(chunk_indices.size());
        for (int idx : chunk_indices) {
            chunk_imgs.push_back(images[idx]);
        }
        processChunk(chunk_imgs, current_run_folder, mode, max_dim, preprocess);
    }

    // 6. Update processed count and free memory
    processed_image_count = static_cast<int>(all_filenames.size());
    images.clear();
}

// -----------------------------------------------------------------------------
// secondPass()
// -----------------------------------------------------------------------------
void Mapping::secondPass(const std::string& run_subdir, cv::Stitcher::Mode mode, int max_dim,
                         bool preprocess) {
    std::string folder_to_scan = current_run_folder.empty() ? run_subdir : current_run_folder;

    // Identify all chunk files (e.g., chunk_*.jpg)
    std::vector<std::string> chunk_files;
    for (const auto& entry : fs::directory_iterator(folder_to_scan)) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();
        if (filename.rfind("chunk_", 0) == 0) {
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
                chunk_files.push_back(entry.path().string());
            }
        }
    }

    if (chunk_files.empty()) {
        std::cerr << "No chunk images found in " << folder_to_scan << ". Nothing to stitch.\n";
        return;
    }

    // Load chunk images (assumed to be already preprocessed)
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

    std::cout << "Loaded " << chunk_images.size() << " chunk images from " << folder_to_scan
              << ". Attempting final stitch...\n";

    // Stitch without additional preprocessing (images are already preprocessed)
    auto [status, final_stitched] = globalStitch(chunk_images, mode, max_dim, preprocess);

    if (status == cv::Stitcher::OK && !final_stitched.empty()) {
        std::string final_name = folder_to_scan + "/final_" + currentDateTimeStr() + ".jpg";
        if (cv::imwrite(final_name, final_stitched)) {
            std::cout << "Final image saved to: " << final_name << "\n";
        } else {
            std::cerr << "Failed to save final image to " << final_name << "\n";
        }
    } else {
        std::cerr << "Final stitching failed. Status = " << static_cast<int>(status) << "\n";
    }

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
    current_run_folder.clear();
    std::cout << "Mapping state has been reset.\n";
}
