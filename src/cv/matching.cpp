#include "cv/matching.hpp"

// https://stackoverflow.com/questions/17533101/resize-a-matrix-after-created-it-in-opencv
auto toInput(cv::Mat img, bool show_output=false, bool unsqueeze=false, int unsqueeze_dim = 0)
{
    cv::Mat img_dst;
    // scale the image and turn into Tensor, then inputs
    cv::resize(img, img_dst, cv::Size(128, 128), 0, 0, cv::INTER_AREA);
    at::Tensor tensor_image = torch::from_blob(img_dst.data, { 3, img_dst.rows, img_dst.cols }, at::kByte);
    // convert back to CV image and display to verify fidelity of image.
    if (unsqueeze)
        tensor_image.unsqueeze_(unsqueeze_dim);
    
    if (show_output)
        std::cout << tensor_image.slice(2, 0, 1) << std::endl;

    return std::vector<torch::jit::IValue>{tensor_image};
}

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
            //cv::Mat image = referenceImages.at(i).croppedImage;
            cv::Mat image = referenceImages.at(i);
            std::vector<torch::jit::IValue> input = toInput(image);

            at::Tensor output = this->module.forward(input).toTensor();
            this->referenceFeatures.push_back(output);
        }
    
    }

// NOTE: Assumes index in referenceImages == index of bottle == index in referenceFeatures
// Further, assumes size of referenceImages == NUM_AIRDROP_BOTTLES
MatchResult Matching::match(const CroppedTarget& croppedTarget) {
    std::vector<torch::jit::IValue> input = toInput(croppedTarget.croppedImage);
    at::Tensor output = this->module.forward(input).toTensor();
    double minDist = this->matchThreshold + 1;
    int minIndex = 0;
    for(int i = 0; i < referenceFeatures.size(); i++) {
        at::Tensor dist = at::pairwise_distance(output, referenceFeatures.at(i));
        double tmp = dist.item().to<double>();
        if(tmp < minDist) {
            minDist = tmp;
            minIndex = i;
        }
    }
    bool isMatch = minDist < this->matchThreshold;
    return MatchResult{minIndex, isMatch, minDist};
}
