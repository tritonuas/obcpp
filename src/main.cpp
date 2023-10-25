#include <iostream>

#include "core/states.hpp"

#include <opencv2/opencv.hpp>
// #include <torch/torch.h>
#include <iostream>

int main() {
    std::cout << "hello" << std::endl;

    PreparationState state;

    state.tick();

//   torch::Tensor tensor = torch::rand({2, 3});
//   std::cout << tensor << std::endl;
    return 0;
}