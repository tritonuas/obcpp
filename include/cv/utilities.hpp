#ifndef INCLUDE_CV_UTILITIES_HPP_
#define INCLUDE_CV_UTILITIES_HPP_

#include <opencv2/opencv.hpp>

class Bbox {
 public:
    int x1;
    int y1;
    int x2;
    int y2;

    int width();
    int height();
};

struct CroppedTarget {
    cv::Mat croppedImage;
    Bbox bbox;
    bool isMannikin;
};

cv::Mat crop(cv::Mat original, Bbox bbox);

#endif  // INCLUDE_CV_UTILITIES_HPP_
