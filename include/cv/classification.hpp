#ifndef INCLUDE_CV_CLASSIFICATION_HPP_
#define INCLUDE_CV_CLASSIFICATION_HPP_

#include "cv/utilities.hpp"
#include <torch/torch.h>
#include "cv/segmentation.hpp"

#include <map>


struct ClassificationResults {
    // TODO: replace with protobuf structs instead of strings
    std::string shape;
    std::string shapeColor;
    std::string character;
    std::string characterColor;
};

// Classification is responsible for predicting characteristics about
// ground comptition targets. These characterisitcs include shape type,
// alphanumeric character type, shape color, and alphanumeric character color.
// Currently, the shape and character classifiers are implemented using a
// Convolutional Neural Network (CNN).
// The color classifier is implented using a K-nearest neigbors model (KNN)
// The implementation of the models themselves can be found here:
// https://github.com/tritonuas/taxonomy-101
// In this class we will take the pretrained models and use them to make
// inferences.
class Classification {
 public:
    Classification() {}
    explicit Classification(const std::string &shapeModelPath, \
        const std::string &characterModelPath, const std::string &colorModelPath);
    // classify takes a cropped image of the target (saliency output) and
    // two binary masks to represent which region of pixels correspond to
    // shape and character respectivel (output of segmentation). Using this
    // data, the shape type, character type, shape color and character color
    // will be predicted.
    ClassificationResults classify(cv::Mat croppedImage, cv::Mat shapeMask, cv::Mat characterMask);

 private:
    torch::jit::script::Module shapeModel;
    torch::jit::script::Module characterModel;
    torch::jit::script::Module colorModel;
    std::map<int, std::string> index_to_shape;

    // classifyShape takes a cropped image of the target (output of saliency)
    // and a binary mask (output of segmentation). The binary mask should
    // represent which region of pixels correspond to the shape region of
    // the target.
    std::string classifyShape(cv::Mat croppedImage, cv::Mat shapeMask);

    // classifyShape takes a cropped image of the target (output of saliency)
    // and a binary mask (output of segmentation). The binary mask should
    // represent which region of pixels correspond to the character region of
    // the target.
    std::string classifyCharacter(cv::Mat croppedImage, cv::Mat characterMask);

    // classify the primary color of a region described by a binary mask.
    // This can be used for finding both shape and character color since
    // we will use the same algorithm to detect the primary color in
    // whatever region the mask describes. All that changes is the mask
    // that's passed in.
    std::string classifyColor(cv::Mat croppedImage, cv::Mat mask);
};


#endif  // INCLUDE_CV_CLASSIFICATION_HPP_
