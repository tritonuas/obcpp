#include <torch/torch.h>
#include <torch/script.h>
#include <torchvision/vision.h>
#include <iostream>

#include <loguru.hpp>

/*
* Test loading a model that uses torchvision operators 
* bin/load_torchvision_model [model_file] 
* can download model_file from https://drive.google.com/file/d/1q_33E7IDbfLS4x3FM21Poy3NbiHS0WtW/view?usp=drive_link
*/ 
int main (int argc, char *argv[]) {
    if (argc < 2) {
        LOG_F(ERROR, "did not provide model_file as argument");
        return 1;
    }
    torch::jit::script::Module module;
    try {
        module = torch::jit::load(argv[1]);
    } catch (const c10::Error& e) {
        LOG_S(ERROR) << "error loading the model: : " << e.msg() << "\n";
        return 1;
    }

    LOG_F(INFO, "loaded model without crashing");
    return 0;
}