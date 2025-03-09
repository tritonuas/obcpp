#ifndef INCLUDE_CV_PREPROCESS_HPP_
#define INCLUDE_CV_PREPROCESS_HPP_

#include <opencv2/core.hpp>

// Preprocess chops off 20px from the right side of the image to
// account for the green bar from the camera
class Preprocess {
 public:
    Preprocess() = default;

    // Crops 20 pixels from the right side of the provided image.
    // Returns a new cv::Mat containing the cropped image.
    cv::Mat cropRight(const cv::Mat &image) const;

 private:
    const int crop_pixels_ = 20;
};

#endif  // INCLUDE_CV_PREPROCESS_HPP_
