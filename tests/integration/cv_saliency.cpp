#include <torch/script.h> // One-stop header.
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "cv/saliency.hpp"
#include "utilities/common.hpp"
#include "loguru.hpp"

// expected arguments: <path-to-model> <path-to-image> 
int main(int argc, const char* argv[]) {
 
  if (argc != 3) {
    std::cerr << "usage: example-app <path-to-model> <path-to-image>\n";
    return -1;
  }

  // convert image to tensor
 
  const char* modelPath = argv[1];
  auto startLoad = getUnixTime_ms().count();
  Saliency sal(modelPath);
  auto endLoad = getUnixTime_ms().count();
  LOG_F(INFO, "time to load: %u milliseconds", endLoad - startLoad);

  const char* imgPath = argv[2];
  cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);

  auto startSalience = getUnixTime_ms().count();
  std::vector<CroppedTarget> predictions = sal.salience(img); 
  auto endSalience = getUnixTime_ms().count();
  LOG_F(INFO, "time to saliency: %u milliseconds", endSalience - startSalience);
  
  img = cv::imread(imgPath, cv::IMREAD_COLOR);
  
  for (CroppedTarget ct: predictions) {
    cv::Rect roi; // setup region of interest (cropped target)
    roi.x = ct.bbox.x1;
    roi.y = ct.bbox.y1;
    roi.width = ct.bbox.x2 - ct.bbox.x1;
    roi.height = ct.bbox.y2 - ct.bbox.y1;
    cv::rectangle(img, roi, (cv::Scalar) (0,255,0), 2); 
  }    

  // cv::namedWindow("cropped targets", cv::WINDOW_FULLSCREEN);
  // cv::imshow("cropped targets", img);
  // cv::waitKey(0);  
 
  cv::imwrite("croppedTargets.jpg", img);
  LOG_F(INFO, "saved croppedTargets.jpg to build/");
  // testing: save input image to file path (cv::imsave?) with bounding boxes overlayed

  // std::cout << "results:\n";
  // for (int i = 0; i < targets.size(); i++) {
  //     std::cout << "box: \n";
  //     std::cout << targets[i].bbox.x1 << ", " << targets[i].bbox.y1 << ", " << targets[i].bbox.x2 << ", " << targets[i].bbox.y2 << "\n"; 
  //     std::cout << "isMannikin = " << targets[i].isMannikin;
  // }

}