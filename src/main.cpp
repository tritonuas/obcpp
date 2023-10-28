#include <iostream>

#include "Eigen"
#include <torch/torch.h>

#include <iostream>
#include <httplib.h>

#include "core/states.hpp"
#include "pathing/plotting.hpp"

int main()
{
    httplib::Server svr;
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("0.0.0.0", 8080);
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>

int main() {
    std::vector<std::string> img_fn = {"../imgs/img1.jpeg", "../imgs/img2.jpeg", "../imgs/img3.jpeg", "../imgs/img4.jpeg"};
    std::vector<cv::Mat> img_list;
    for (const std::string& fn : img_fn) {
        img_list.push_back(cv::imread(fn));
    }

    cv::Mat exposure_times = (cv::Mat_<float>(1, 4) << 15.0, 2.5, 0.25, 0.0333);
    
    std::vector<cv::Mat> img_aligned = img_list;
/*
    // Create an Aligner object for feature-based image alignment
    cv::Ptr<cv::AlignExposures> aligner = cv::AlignExposures();

    // Align images
    cv::Mat img_aligned;
    aligner->process(img_list, img_aligned, exposure_times);
*/
    // Merge exposures to HDR image using Debevec method
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    cv::Mat hdr_debevec;
    merge_debevec->process(img_aligned, hdr_debevec, exposure_times);

    // Merge exposures to HDR image using Robertson method
    cv::Ptr<cv::MergeRobertson> merge_robertson = cv::createMergeRobertson();
    cv::Mat hdr_robertson;
    merge_robertson->process(img_aligned, hdr_robertson, exposure_times);

    // Tonemap HDR image
    cv::Ptr<cv::Tonemap> tonemap1 = cv::createTonemap(2.2);
    cv::Mat res_debevec;
    tonemap1->process(hdr_debevec, res_debevec);

    // Exposure fusion using Mertens
    cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
    cv::Mat res_mertens;
    merge_mertens->process(img_aligned, res_mertens);

    // Convert datatype to 8-bit and save
    cv::Mat res_debevec_8bit;
    res_debevec.convertTo(res_debevec_8bit, CV_8U, 255.0);

    cv::Mat res_robertson_8bit;
    hdr_robertson.convertTo(res_robertson_8bit, CV_8U, 255.0);

    cv::Mat res_mertens_8bit;
    res_mertens.convertTo(res_mertens_8bit, CV_8U, 255.0);

    cv::imwrite("ldr_debevec.jpg", res_debevec_8bit);
    cv::imwrite("ldr_robertson.jpg", res_robertson_8bit);
    cv::imwrite("fusion_mertens.jpg", res_mertens_8bit);
    return 0;

/*
  std::cout << "hellasdfasdfTarget torch_cpu not found.o" << std::endl;

  PreparationState state;

  state.tick();
  
  return 0;
*/
}