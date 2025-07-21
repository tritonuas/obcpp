#include "cv/utilities.hpp"

int Bbox::width() const { return x2 - x1; }

int Bbox::height() const { return y2 - y1; }

// crop will crop an OpenCV image and return a new image cropped to the region
// specified by the given bounding box
cv::Mat crop(const cv::Mat& original, const Bbox& bbox) {
    auto x = cv::Mat(original.clone(), cv::Rect(bbox.x1, bbox.y1, bbox.width(), bbox.height()));
    return x;
}
