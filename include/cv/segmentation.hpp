#ifndef INCLUDE_CV_SEGMENTATION_HPP_
#define INCLUDE_CV_SEGMENTATION_HPP_

#include <opencv2/opencv.hpp>

#include <torch/script.h>
#include <iostream>
#include <memory>

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
        Segmentation() {
            // Initialize without a model or set up default values
        }

        Segmentation(const std::string &modelPath) {
            try {
                this->module = torch::jit::load(modelPath);
            }
            catch (const c10::Error& e) {
                std::cerr << "ERROR: could not load the model, check if model file is present in /bin\n";
                throw;
            }
        }
        SegmentationResults segment(const CroppedTarget &target);
    private:
        torch::jit::script::Module module;
};

std::string get_image_type(const cv::Mat& img, bool more_info);
void show_image(cv::Mat& img, std::string title);
at::Tensor transpose(at::Tensor tensor, c10::IntArrayRef dims);
std::vector<torch::jit::IValue> ToInput(at::Tensor tensor_image);
at::Tensor ToTensor(cv::Mat img, bool show_output, bool unsqueeze, int unsqueeze_dim);
cv::Mat ToCvImage(at::Tensor tensor);

#endif  // INCLUDE_CV_SEGMENTATION_HPP_
