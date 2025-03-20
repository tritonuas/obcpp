#ifndef INCLUDE_CV_UTILITIES_HPP_
#define INCLUDE_CV_UTILITIES_HPP_

#include <opencv2/opencv.hpp>

#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

class Bbox {
 public:
    int x1;
    int y1;
    int x2;
    int y2;

    int width();
    int height();
};

struct DetectedTarget {
    Bbox bbox;       // The bounding box in pixel coords
    GPSCoord coord;  // The localized GPS coordinate
    BottleDropIndex likely_bottle;
    double match_distance;  // Inverse confidence or other distance metric
};

// Helper function to crop out a bounding box from an image if you still need it
cv::Mat crop(const cv::Mat& original, const Bbox& bbox);

#endif  // INCLUDE_CV_UTILITIES_HPP_
