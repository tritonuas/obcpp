#ifndef INCLUDE_CV_SALIENCY_HPP_
#define INCLUDE_CV_SALIENCY_HPP_

#include <opencv2/opencv.hpp>

#include "cv/utilities.hpp"

// Saliency is responsible for detecting targets within a full-size aerial image
// It can detect the presence of standard targets (with shape and character)
// as well as emergent targets (mannikin). It outputs the position of the
// targets within the image (using coordinates of a bounding box) and a
// classification between if a target is standard or emergent. Saliency is
// implemented as a convolutional neural network using the Faster RCNN
// model. The training/testing code for the model can be found here:
// https://github.com/tritonuas/garretts-new-lunchbox
class Saliency {
    public:
        // saliency takes a full-size aerial image and detects any potential
        // targets within the image. The full-size image can have multiple
        // targets which is why it returns a vector of targets. For each target,
        // it will predict the location within the full-size image (using
        // coordinates of bounding box) and a prediction of whether or not a
        // target is emergent (mannikin) or not.
        std::vector<CroppedTarget> salience(cv::Mat image);
};

#endif  // INCLUDE_CV_SALIENCY_HPP_