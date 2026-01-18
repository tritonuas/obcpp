#include "cv/utilities.hpp"
#include "utilities/logging.hpp"

int Bbox::width() const { return x2 - x1; }

int Bbox::height() const { return y2 - y1; }

// crop will crop an OpenCV image and return a new image cropped to the region
// specified by the given bounding box
cv::Mat crop(const cv::Mat& original, const Bbox& bbox) {
    auto x = cv::Mat(original.clone(), cv::Rect(bbox.x1, bbox.y1, bbox.width(), bbox.height()));
    return x;
}


// Compresses a JPEG image to a desired quality and also denoises it.
// Returns std::nullopt if the image compression fails. 
// A quality of about 60 compresses an image to almost half the file size.
std::optional<cv::Mat> compressImg(const cv::Mat& img, int quality) {
    std::vector<int> compressionParams {cv::IMWRITE_JPEG_QUALITY, quality};
    std::vector<uchar> buffer;
    cv::Mat denoisedImg;

    cv::fastNlMeansDenoisingColored(img, denoisedImg);

    if (!cv::imencode(".jpg", denoisedImg, buffer, compressionParams)) {
        LOG_F(WARNING, "Unable to encode the image into a buffer. Using the orignal instead");
        return std::nullopt;
    }

    cv::Mat decodedImg = cv::imdecode(buffer, cv::IMREAD_COLOR);

    if (decodedImg.empty()) {
        LOG_F(WARNING, "Unable to decoode the image from a buffer. Using the orignal instead");
        return std::nullopt;
    }

    return std::make_optional(decodedImg);
}
