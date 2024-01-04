#ifndef INCLUDE_CV_SEGMENTATION_HPP_
#define INCLUDE_CV_SEGMENTATION_HPP_

#include <opencv2/opencv.hpp>

#include "cv/utilities.hpp"

struct SegmentationResults {
    cv::Mat shapeMask;
    cv::Mat characterMask;
};

// Segmentation is the stage of the pipeline responsible for detecting the
// region of a cropped target that corresponds to the shape/character
// attributes. This is useful for simplifying the job of the classifcation model.
// Assuming that segmentation is accurate, classification only needs to look at
// the binary mask of the target's shape and character. This simplifies
// classification to a simple MNIST classification problem where we need to
// classify a black and white shape/character image. The training/testing code
// for segmentation models can be found here:
// https://github.com/tritonuas/hutzler-571
class Segmentation {
    public:
       SegmentationResults segment(const CroppedTarget &target);
};

#endif  // INCLUDE_CV_SEGMENTATION_HPP_