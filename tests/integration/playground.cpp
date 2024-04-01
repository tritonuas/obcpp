#include <iostream>
#include <string>

// #include "ArenaApi.h"
#include <torch/torch.h>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <matplot/matplot.h>
#include <mavsdk/mavsdk.h>
#include <loguru.hpp>

// using json = nlohmann::json;

int main (int argc, char *argv[]) {
    // test opencv
    LOG_F(INFO, "Testing opencv installation\n");
    cv::Mat opencv_mat = cv::Mat::eye(300, 300, CV_32F);
    LOG_F(INFO, "Rows: %d Cols: %d\n", opencv_mat.rows, opencv_mat.cols);
    
    // test matplot
    LOG_F(INFO, "Testing matplot installation\n");
    matplot::color c = matplot::color::black;
    LOG_F(INFO, "Black: %d\n", static_cast<int>(c));
    
    // test mavsdk
    LOG_F(INFO, "Testing mavsdk installation\n");
    mavsdk::Mavsdk mavsdk;
    std::string connection_address = "tcp://127.0.0.1:5760";
    mavsdk::ConnectionResult conn_result = mavsdk.add_any_connection(connection_address);
    LOG_F(INFO, "Expected connection to be refused\n");
    
    return 0;
}