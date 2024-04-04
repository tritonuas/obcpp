#ifndef INCLUDE_CV_UTILITIES_HPP_
#define INCLUDE_CV_UTILITIES_HPP_

#include <opencv2/opencv.hpp>

#include <array>

#include "utilities/constants.hpp"
#include "protos/obc.pb.h"

struct DetectedTarget {
    DetectedTarget(GPSCoord coord, BottleDropIndex index, double match_distance)
        :coord{coord}, index{index}, match_distance{match_distance} {}

    GPSCoord coord;
    BottleDropIndex index;
    double match_distance; 
};

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
