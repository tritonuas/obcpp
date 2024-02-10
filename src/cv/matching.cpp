#include "cv/matching.hpp"

/*
* IMPORTANT NOTE:
* Using torch::from_blob to convert cv::Mat to torch::Tensor results in NaN tensors 
* for certain images. The solution of using std::memcpy instead was found and corroborated 
* in this post: 
* https://discuss.pytorch.org/t/convert-torch-tensor-to-cv-mat/42751/4
*
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
auto toInput(cv::Mat img, bool show_output=false, bool unsqueeze=false, int unsqueeze_dim = 0)
{
    cv::Mat img_dst;
    // scale the image and turn into Tensor, then inputs
    cv::resize(img, img_dst, cv::Size(128, 128), 0, 0, cv::INTER_AREA);
    img_dst.convertTo(img_dst, CV_32FC3);
    //torch::Tensor tensor_image = torch::from_blob(img_dst.data, {1, 3, img_dst.rows, img_dst.cols}, torch::kF32);
    torch::Tensor tensor_image = torch::ones({1, 3, img_dst.rows, img_dst.cols}, torch::kF32);
    std::memcpy(tensor_image.data_ptr(), img_dst.data, sizeof(float)*tensor_image.numel());
    // convert back to CV image and display to verify fidelity of image.
    if (unsqueeze)
        tensor_image.unsqueeze_(unsqueeze_dim);
        
    
    if (show_output)
        std::cout << tensor_image.slice(2, 0, 1) << std::endl;

    return std::vector<torch::jit::IValue>{tensor_image};
}
// change referenceImages to a vector of pairs. In each pair: cv::Mat image, uint8_t bottleIndex
// So each reference image also comes with the bottle index it corresponds to.
Matching::Matching(std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES>
                       competitionObjectives,
                   double matchThreshold,
                   std::vector<cv::Mat> referenceImages,
                   const std::string &modelPath)
    : competitionObjectives(competitionObjectives),
      matchThreshold(matchThreshold) {

        try {
            this->module = torch::jit::load(modelPath);
        }
        catch (const c10::Error& e) {
            std::cerr << "error loading the model\n";
            throw;
        }

        for(int i = 0; i < referenceImages.size(); i++) {
            cv::Mat image = referenceImages.at(i);
            std::vector<torch::jit::IValue> input = toInput(image);
            torch::Tensor output = this->module.forward(input).toTensor();
            this->referenceFeatures.push_back(output);
        }
    
    }

// NOTE: Assumes index in referenceImages == index of bottle == index in referenceFeatures
// Further, assumes size of referenceImages == NUM_AIRDROP_BOTTLES
MatchResult Matching::match(const CroppedTarget& croppedTarget) {
    std::vector<torch::jit::IValue> input = toInput(croppedTarget.croppedImage);
    torch::Tensor output = this->module.forward(input).toTensor();
    uint8_t minIndex = 0;
    double minDist = torch::pairwise_distance(output, referenceFeatures.at(minIndex)).item().to<double>();
    for(uint8_t i = 0; i < referenceFeatures.size(); i++) {
        torch::Tensor curr = referenceFeatures.at(i);
        torch::Tensor dist = torch::pairwise_distance(output, curr);
        double tmp = dist.item().to<double>();
        if(tmp < minDist) {
            minDist = tmp;
            minIndex = i;
        }
    }
    bool isMatch = minDist < this->matchThreshold;
    return MatchResult{minIndex, isMatch, minDist};
}
