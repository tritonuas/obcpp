#include "cv/matching.hpp"

#include <loguru.hpp>

/*
* IMPORTANT NOTE:
* Using torch::from_blob to convert cv::Mat to torch::Tensor results in NaN tensors 
* for certain images. The solution of using std::memcpy instead was found and corroborated 
* in this post: 
* https://discuss.pytorch.org/t/convert-torch-tensor-to-cv-mat/42751/4
* The old broken line is given here for reference (formerly at line 33): 
* torch::Tensor tensor_image = torch::from_blob(img_dst.data, {1, 3, img_dst.rows, img_dst.cols}, torch::kF32);

* The reason for from_blob's misbehaving tendencies may have to do with it taking in
* a pointer as its first argument (img_dst.data) as shown here:
* https://pytorch.org/cppdocs/api/function_namespacetorch_1ad7fb2a7759ef8c9443b489ddde494787.html

* Every time we exit toInput(), img_dst is deallocated and the pointer to the image's data
* is thus rendered invalid. This results in tensors filled with NaN.
* This hypothesis is corroborated by the following post:
* https://discuss.pytorch.org/t/std-vector-torch-tensor-turns-into-nans/81853 
* If we wanted to use from_blob, we would have to perhaps pass in a constant reference to
* img, and only use img (rather than creating a new img_dst to do operations on).
* Std::memcpy works for the time being, so it will remain.
*
* Reference for resizing cv image: 
* https://stackoverflow.com/questions/17533101/resize-a-matrix-after-created-it-in-opencv
*/
auto toInput(cv::Mat img, bool show_output = false, bool unsqueeze = false, int unsqueeze_dim = 0) {
    cv::Mat img_dst;
    // scale the image and turn into Tensor, then input vector
    cv::resize(img, img_dst, cv::Size(128, 128), 0, 0, cv::INTER_AREA);
    img_dst.convertTo(img_dst, CV_32FC3);
    torch::Tensor tensor_image = torch::ones({1, 3, img_dst.rows, img_dst.cols}, torch::kF32);
    std::memcpy(tensor_image.data_ptr(), img_dst.data, sizeof(float)*tensor_image.numel());
    // convert back to CV image and display to verify fidelity of image if desired
    if (unsqueeze)
        tensor_image.unsqueeze_(unsqueeze_dim);

    if (show_output) {
        std::ostringstream stream;
        stream << tensor_image.slice(2, 0, 1);
        LOG_F(INFO, stream.str().c_str());
    }

    return std::vector<torch::jit::IValue>{tensor_image};
}

Matching::Matching(std::array<Bottle, NUM_AIRDROP_BOTTLES>
                       competitionObjectives,
                   double matchThreshold,
                   std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages,
                   const std::string &modelPath)
    : competitionObjectives(competitionObjectives),
      matchThreshold(matchThreshold) {

    try {
        this->module = torch::jit::load(modelPath);
    }
    catch (const c10::Error& e) {
        LOG_F(ERROR, "ERROR: could not load the model, check if model file is present in /bin\n");
        throw;
    }

    if (referenceImages.size() == 0) {
        LOG_F(ERROR, "WARNING: Empty reference image vector passed as argument\n");
    }

    // Populate referenceFeatures by putting each refImg through model
    for (int i = 0; i < referenceImages.size(); i++) {
        cv::Mat image = referenceImages.at(i).first;
        BottleDropIndex bottle = referenceImages.at(i).second;
        std::vector<torch::jit::IValue> input = toInput(image);
        torch::Tensor output = this->module.forward(input).toTensor();
        this->referenceFeatures.push_back(std::make_pair(output, bottle));
    }
}

/*
* Matches the given image with one of the referenceFeature elements 
* (each referenceFeature element corresponds to an element of referenceImages, see constructor).
* @param croppedTarget - wrapper for cv::Mat image which we extract and use
* @return MatchResult - struct of bottleIndex, boolean isMatch, and minimum distance found.
* Boolean isMatch set to true if minimum distance found is below threshold, false otherwise.
*
* NOTE: Matching only occurs if loading model and ref. images was successful.
*/
MatchResult Matching::match(const CroppedTarget& croppedTarget) {
    std::vector<torch::jit::IValue> input = toInput(croppedTarget.croppedImage);
    torch::Tensor output = this->module.forward(input).toTensor();
    int minIndex = 0;
    double minDist = torch::pairwise_distance(output,
                            referenceFeatures.at(minIndex).first).item().to<double>();

    for (int i = 1; i < referenceFeatures.size(); i++) {
        torch::Tensor curr = referenceFeatures.at(i).first;
        torch::Tensor dist = torch::pairwise_distance(output, curr);
        double tmp = dist.item().to<double>();
        if (tmp < minDist) {
            minDist = tmp;
            minIndex = i;
        }
    }
    BottleDropIndex bottleIdx = referenceFeatures.at(minIndex).second;
    bool isMatch = minDist < this->matchThreshold;
    return MatchResult{bottleIdx, isMatch, minDist};
}
