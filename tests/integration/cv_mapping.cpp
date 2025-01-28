#include <iostream>
#include <string>

#include "cv/mapping.hpp"

int main() {
    // Input directory where new images appear
    std::string input_dir = "../tests/integration/images";

    // Directory to save chunk results (and eventually final)
    std::string output_dir = "../tests/integration/output";

    // Stitching parameters
    const int chunk_size = 5;
    const int overlap = 2;
    const int max_dim = 5000;

    // Create the mapper
    Mapping mapper;

    // First pass (can be called multiple times if you receive images in bursts)
    mapper.firstPass(input_dir,   // Where images are located
                     output_dir,  // Where to save chunk results
                     chunk_size, overlap, cv::Stitcher::SCANS, max_dim);

    // If more images arrive, we can call firstPass(...) again.
    // ...
    // Suppose no more images are coming, so do the second pass.

    mapper.secondPass(output_dir, cv::Stitcher::SCANS, max_dim);

    return 0;
}
