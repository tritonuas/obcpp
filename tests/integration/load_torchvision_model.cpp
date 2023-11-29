#include <torch/torch.h>
#include <torch/script.h>
#include <torchvision/vision.h>
#include <iostream>

int main (int argc, char *argv[]) {
    torch::jit::script::Module module;
    try {
        // model can be downloaded with https://drive.google.com/file/d/1q_33E7IDbfLS4x3FM21Poy3NbiHS0WtW/view?usp=drive_link
        module = torch::jit::load("../default.pth");
    } catch (const c10::Error& e) {
        std::cerr << "error loading the model: : " << e.msg() << std::endl;
        return 1;
    }

    std::cout << "loaded model without crashing" << std::endl;
    return 0;
}