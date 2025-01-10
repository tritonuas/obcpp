#include "cv/mapping.hpp"

int main() {
    // Assuming the executable is in /build/bin
    std::string input_dir = "../../tests/integration/images";
    std::string output_dir = "../../tests/integration/output";

    Mapping mapper;
    mapper.mapImages(input_dir, output_dir);
    return 0;
}