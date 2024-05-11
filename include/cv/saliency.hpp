#ifndef INCLUDE_CV_SALIENCY_HPP_
#define INCLUDE_CV_SALIENCY_HPP_
#include <torch/torch.h>
#include <vector>
#include <string>
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

    explicit Saliency(std::string modelPath);   // constructor with model path

    std::vector<CroppedTarget> salience(cv::Mat image);     // saliency function

    // helper functions
    at::Tensor ToTensor(cv::Mat img, bool show_output, bool unsqueeze, int unsqueeze_dim);
    at::Tensor transpose(at::Tensor tensor, c10::IntArrayRef dims);
    std::vector<torch::jit::IValue> ToInput(at::Tensor tensor_image);
    std::vector<CroppedTarget> extractTargets(c10::List<c10::IValue> listDetections, cv::Mat image);

 private:
    std::string modelPath;              // path to prediction model
    torch::jit::script::Module module;  // the loaded model
};

#endif  // INCLUDE_CV_SALIENCY_HPP_
