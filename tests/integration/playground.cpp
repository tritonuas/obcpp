#include <iostream>
#include <string>

// #include "ArenaApi.h"
#include <torch/torch.h>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <matplot/matplot.h>
#include <mavsdk/mavsdk.h>

// using json = nlohmann::json;

int main (int argc, char *argv[]) {
    // test opencv
    std::cout << "Testing opencv installation" << std::endl;
    cv::Mat opencv_mat = cv::Mat::eye(300, 300, CV_32F);
    std::cout << "Rows: " << opencv_mat.rows << " Cols: " << opencv_mat.cols << std::endl;
    
    // test matplot
    std::cout << "Testing matplot installation" << std::endl;
    matplot::color c = matplot::color::black;
    std::cout << "Black: " << static_cast<int>(c) << std::endl;
    
    // test mavsdk
    std::cout << "Testing mavsdk installation" << std::endl;
    mavsdk::Mavsdk mavsdk;
    std::string connection_address = "tcp://127.0.0.1:5760";
    mavsdk::ConnectionResult conn_result = mavsdk.add_any_connection(connection_address);
    std::cout << "Expected connection to be refused" << std::endl;
    
    return 0;
}