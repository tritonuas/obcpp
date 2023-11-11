#include <iostream>

#include "ArenaApi.h"
#include <torch/torch.h>
#include <nlohmann/json.hpp>
#include <Eigen>
#include <opencv2/opencv.hpp>

using json = nlohmann::json;

int main (int argc, char *argv[]) {
    // test arena
    // Arena::ISystem* pSystem = Arena::OpenSystem();

    // test torch
    std::cout << "Testing torch installation" << std::endl;
    torch::Tensor tensor = torch::eye(3);
    std::cout << tensor << "\n" << std::endl;

    // test json
    std::cout << "Testing json installation" << std::endl;
    json data = {
        {"json", true},
        {"works", true},
    };
    std::cout << data << "\n" << std::endl;

    // test eigen
    std::cout << "Testing eigen installation" << std::endl;
    Eigen::Vector3d vec = Eigen::Vector3d(1, 2, 3);
    std::cout << vec << "\n" << std::endl;

    // test opencv
    std::cout << "Testing opencv installation" << std::endl;
    cv::Mat opencv_mat = cv::Mat::eye(300, 300, CV_32F);
    std::cout << "Rows: " << opencv_mat.rows << " Cols: " << opencv_mat.cols << std::endl;
    
    return 0;
}