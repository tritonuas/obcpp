#ifndef INCLUDE_CV_UTILITIES_HPP_
#define INCLUDE_CV_UTILITIES_HPP_

#include <array>

#include <opencv2/opencv.hpp>

#include "utilities/constants.hpp"
#include "protos/obc.pb.h"

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

struct DetectedTarget {
    DetectedTarget(GPSCoord coord, BottleDropIndex index, double match_distance, CroppedTarget crop)
        :coord{coord}, likely_bottle{index}, match_distance{match_distance}, crop{crop} {}

    GPSCoord coord;
    BottleDropIndex likely_bottle;
    double match_distance;
    CroppedTarget crop;
};

cv::Mat crop(cv::Mat original, Bbox bbox);

#endif  // INCLUDE_CV_UTILITIES_HPP_
