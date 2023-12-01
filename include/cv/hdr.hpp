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

/**
 * Load real images from imgs/
*/
std::vector<cv::Mat> load_images();

/**
 * Experimental function which implements a few versions of HDR.
*/
int compute_hdr(std::vector<cv::Mat> img_list, const cv::Mat& exposure_times);

/**
 * Return exposure times for each image. Needed for HDR algorithm.
*/
cv::Mat get_image_exposure_times();

#endif