#include <torch/torch.h>
#include <torch/script.h>
#include <torchvision/vision.h>
#include <iostream>

/*
* Test loading a model that uses torchvision operators 
* bin/load_torchvision_model [model_file] 
* can download model_file from https://drive.google.com/file/d/1q_33E7IDbfLS4x3FM21Poy3NbiHS0WtW/view?usp=drive_link
*/ 
int main (int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "did not provide model_file as argument" << std::endl;
        return 1;
    }
    torch::jit::script::Module module;
    try {
        module = torch::jit::load(argv[1]);
    } catch (const c10::Error& e) {
        std::cerr << "error loading the model: : " << e.msg() << std::endl;
        return 1;
    }

    if (torch::cuda::is_available()) {
	    module.to(torch::kCUDA);
	    std::cout << "successfully moved model to GPU (CUDA)" << std::endl;
    }

    std::cout << "loaded model without crashing" << std::endl;
    return 0;
}
