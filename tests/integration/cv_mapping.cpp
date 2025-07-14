#include <filesystem>
#include <iostream>

#include "cv/mapping.hpp"

namespace fs = std::filesystem;

int main() {
    cv::Stitcher::Mode scan_mode = cv::Stitcher::SCANS;
    try {
        // Set up paths based on the folder structure described above
        fs::path base_dir = "../tests/integration/mapping";
        fs::path batch1_dir = base_dir / "batch1";
        fs::path batch2_dir = base_dir / "batch2";
        fs::path live_images_dir = base_dir / "images";  // the "live" folder seen by firstPass()
        fs::path output_dir = base_dir / "output";

        // Make sure the "live" and "output" directories exist (create if they don't)
        if (!fs::exists(live_images_dir)) {
            fs::create_directories(live_images_dir);
        }
        if (!fs::exists(output_dir)) {
            fs::create_directories(output_dir);
        }

        // 1. Simulate receiving Batch 1 images
        // Clear out the live_images_dir if you want to start fresh
        for (const auto& entry : fs::directory_iterator(live_images_dir)) {
            fs::remove_all(entry.path());
        }
        // Copy Batch 1 images into the "images/" folder
        for (const auto& entry : fs::directory_iterator(batch1_dir)) {
            if (entry.is_regular_file()) {
                fs::copy_file(entry.path(), live_images_dir / entry.path().filename(),
                              fs::copy_options::overwrite_existing);
            }
        }

        // 2. Instantiate the mapper and process the first batch
        Mapping mapper;
        const int chunk_size = 5;
        const int chunk_overlap = 2;
        const int max_dim = 3000;

        std::cout << "==> Processing Batch 1...\n";
        // Note: the new last argument 'preprocess' defaults to true.
        mapper.firstPass(live_images_dir.string(), output_dir.string(), chunk_size, chunk_overlap,
                         scan_mode, max_dim, true);

        // 3. Simulate time passing, and receiving Batch 2 images
        // Copy Batch 2 images into "images/"
        for (const auto& entry : fs::directory_iterator(batch2_dir)) {
            if (entry.is_regular_file()) {
                fs::copy_file(entry.path(), live_images_dir / entry.path().filename(),
                              fs::copy_options::overwrite_existing);
            }
        }

        // 4. Process the second batch with the same mapper instance
        std::cout << "==> Processing Batch 2...\n";
        mapper.firstPass(live_images_dir.string(), output_dir.string(), chunk_size, chunk_overlap,
                         scan_mode, max_dim, true);

        // 5. Finally, call secondPass to merge all chunk images in the timestamped run folder
        std::cout << "==> Merging all partial chunk images into final...\n";
        mapper.secondPass(output_dir.string(), scan_mode, max_dim, true);

        std::cout << "Integration test completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception in incremental integration test: " << e.what() << "\n";
        return 1;
    }

    // =========================================================================
    // Test 2: Direct Stitch Integration Test
    // =========================================================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Starting Direct Stitch Integration Test\n";
    std::cout << std::string(60, '=') << "\n";

    try {
        // Set up paths for direct stitch test
        fs::path base_dir = "../tests/integration/mapping";
        fs::path batch1_dir = base_dir / "batch1";
        fs::path batch2_dir = base_dir / "batch2";
        fs::path direct_test_dir = base_dir / "direct_test_images";
        fs::path direct_output_dir = base_dir / "direct_output";

        // Create test directories
        if (!fs::exists(direct_test_dir)) {
            fs::create_directories(direct_test_dir);
        }
        if (!fs::exists(direct_output_dir)) {
            fs::create_directories(direct_output_dir);
        }

        // Clear the direct test directory
        for (const auto& entry : fs::directory_iterator(direct_test_dir)) {
            fs::remove_all(entry.path());
        }

        // Copy ALL images from both batches into the direct test directory
        std::cout << "Copying images from batch1 and batch2 to direct test directory...\n";
        int image_count = 0;

        // Copy from batch1
        if (fs::exists(batch1_dir)) {
            for (const auto& entry : fs::directory_iterator(batch1_dir)) {
                if (entry.is_regular_file()) {
                    fs::copy_file(entry.path(), direct_test_dir / entry.path().filename(),
                                  fs::copy_options::overwrite_existing);
                    image_count++;
                }
            }
        }

        // Copy from batch2
        if (fs::exists(batch2_dir)) {
            for (const auto& entry : fs::directory_iterator(batch2_dir)) {
                if (entry.is_regular_file()) {
                    fs::copy_file(entry.path(), direct_test_dir / entry.path().filename(),
                                  fs::copy_options::overwrite_existing);
                    image_count++;
                }
            }
        }

        std::cout << "Copied " << image_count << " images for direct stitching test.\n";

        if (image_count == 0) {
            std::cout << "No images found in batch directories. Creating a simple test...\n";
            std::cout << "Please add some test images to batch1/ or batch2/ directories.\n";
        } else {
            // Test the direct stitch method
            Mapping direct_mapper;
            const int max_dim = 3000;

            std::cout << "==> Testing directStitch with preprocessing enabled...\n";
            direct_mapper.directStitch(direct_test_dir.string(), direct_output_dir.string(),
                                       scan_mode, max_dim, true);

            std::cout << "==> Testing directStitch with preprocessing disabled...\n";
            direct_mapper.directStitch(direct_test_dir.string(),
                                       (direct_output_dir / "no_preprocess.jpg").string(),
                                       scan_mode, max_dim, false);

            std::cout << "Direct stitch integration test completed.\n";
            std::cout << "Check the following directory for results: " << direct_output_dir << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception in direct stitch integration test: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "All integration tests completed successfully!\n";
    std::cout << std::string(60, '=') << "\n";

    return 0;
}
