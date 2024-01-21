#include "cv/matching.hpp"

// https://stackoverflow.com/questions/17533101/resize-a-matrix-after-created-it-in-opencv
auto toInput(cv::Mat img, bool show_output=false, bool unsqueeze=false, int unsqueeze_dim = 0)
{
    cv::Mat img_dst;
    // scale the image and turn into Tensor, then inputs
    cv::resize(img, img_dst, cv::Size(128, 128), 0, 0, cv::INTER_AREA);
    at::Tensor tensor_image = torch::from_blob(img_dst.data, { img_dst.rows, img_dst.cols, 3 }, at::kByte);
    if (unsqueeze)
        tensor_image.unsqueeze_(unsqueeze_dim);
    
    if (show_output)
        std::cout << tensor_image.slice(2, 0, 1) << std::endl;

    return std::vector<torch::jit::IValue>{tensor_image};
}

// NOTE: Assumes index in referenceImages == index of bottle == index in referenceFeatures
// Further, assumes size of referenceImages == NUM_AIRDROP_BOTTLES
Matching::Matching(std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES>
                       competitionObjectives,
                   double matchThreshold,
                   std::vector<CroppedTarget> referenceImages)
    : competitionObjectives(competitionObjectives),
      matchThreshold(matchThreshold) {

        try {
            this->module = torch::jit::load("../bin/target_siamese_1.pt");
        }
        catch (const c10::Error& e) {
            std::cerr << "error loading the model\n";
            return -1;
        }

        for(int i = 0; i < referenceImages.size(); i++) {
            cv::Mat image = referenceImages.at(i).croppedImage;
            std::vector<torch::jit::IValue> input = toInput(image);

            at::Tensor output = this->module.forward(input).toTensor();
            this->referenceFeatures.push_back(output);
        }
    
    }


MatchResult Matching::match(const CroppedTarget& croppedTarget) {
    std::vector<torch::jit::IValue> input = toInput(croppedTarget.croppedImage);
    at::Tensor output = this->module.forward(input).toTensor();
    double minDist = this->matchTreshold + 1;
    int minIndex = 0;
    for(int i = 0; i < referenceFeatures.size(); i++) {
        double tmp = torch::nn::functional::pairwise_distance(output, 
                                            referenceFeatures.at(i));
        if(tmp < minDist) {
            minDist = tmp;
            minIndex = i;
        }
    }
    bool isMatch = minDist < this->matchThreshold;
    return MatchResult{minIndex, isMatch, minDist};
}
