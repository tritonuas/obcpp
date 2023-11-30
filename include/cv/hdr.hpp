#ifndef CV_HDR_HPP_
#define CV_HDR_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
// #include <opencv2/opencv.hpp>
// #include <opencv2/core.hpp>
// #include <opencv2/photo.hpp>
// #include <opencv2/imgproc.hpp>

/**
 * Generate images for testing.
*/
std::vector<cv::Mat>  generate_test_images();
std::vector<cv::Mat> load_images();
int compute_hdr(std::vector<cv::Mat> img_list, const cv::Mat& exposure_times);
cv::Mat get_image_exposure_times();

#endif