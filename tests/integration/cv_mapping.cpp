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
    return 0;
}
