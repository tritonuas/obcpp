#ifndef INCLUDE_CV_UTILITIES_HPP_
#define INCLUDE_CV_UTILITIES_HPP_

#include <optional>
#include <vector>
#include <opencv2/opencv.hpp>

#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

class Bbox {
 public:
    int x1;
    int y1;
    int x2;
    int y2;

    int width() const;
    int height() const;
};

struct DetectedTarget {
    Bbox bbox;       // The bounding box in pixel coords
    GPSCoord coord;  // The localized GPS coordinate
    AirdropType likely_airdrop;
    double match_distance;  // Inverse confidence or other distance metric
};

// Helper function to crop out a bounding box from an image if you still need it
cv::Mat crop(const cv::Mat& original, const Bbox& bbox);

/**
 * Denoises and compresses a JPEG image to a desired quality.
 * 
 * @param img       : a const reference to the image you want to compress
 * @param quality   : the quality you want to compress to. A quality of about 60
 *                    compresses an image about half size.
 * 
 * @return          : std::nullopt if the compression fails. 
 *                    cv::Mat otherwise
 */
std::optional<cv::Mat> compressImg(const cv::Mat& img, int quality = 60);

#endif  // INCLUDE_CV_UTILITIES_HPP_
