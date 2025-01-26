#include "cv/mapping.hpp"

int main() {
    // Assuming the executable is in /build/bin
    std::string input_dir = "../../tests/integration/images";
    std::string output_dir = "../../tests/integration/output";

    const int chunk_size = 5;

    const int chunk_overlap = 2;
    const int max_dim = 5000;

    Mapping mapper;
    mapper.loadImages(input_dir);
    mapper.mapImages(output_dir, chunk_size, chunk_overlap, max_dim);
    return 0;
}