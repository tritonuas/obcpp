#include "cv/hdr.hpp"

int compute_hdr() {
  std::vector<cv::Mat> img_list;

  // Use generated colors or real images
  if (false) {
    // Uses hard coded local images
    std::vector<std::string> img_fn = {
        "../imgs/img1.jpeg", "../imgs/img2.jpeg", "../imgs/img3.jpeg",
        "../imgs/img4.jpeg", "../imgs/img5.jpeg", "../imgs/img6.jpeg"};
    img_list.reserve(img_fn.size());
    for (const std::string &fn : img_fn) {
      img_list.push_back(cv::imread(fn));
    }
  } else {
    std::vector<cv::Scalar> colors = {
        cv::Scalar(255, 0, 0),   // Blue
        cv::Scalar(0, 255, 0),   // Green
        cv::Scalar(0, 0, 255),   // Red
        cv::Scalar(255, 255, 0), // Cyan
        cv::Scalar(255, 0, 255), // Magenta
        cv::Scalar(0, 255, 255)  // Yellow
    };

    img_list.reserve(colors.size());

    // Generate images of different colors
    for (const auto &color : colors) {
      // Create a 200x200 image filled with the specified color
      cv::Mat colorImg(200, 200, CV_8UC3, color);
      img_list.push_back(colorImg);
    }
  }

  // you need to specify the exposures so HDR knows how to weight the images (in
  // seconds)
  cv::Mat exposure_times = (cv::Mat_<float>(1, 6) << 1 / 174, 1 / 120, 1 / 120,
                            1 / 1374, 1 / 6211, 1 / 12048);

  std::vector<cv::Mat> img_aligned = img_list;

  // TODO: The images must be aligned before the HDR superimposing
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
}
