#include "cv/preprocess.hpp"

#include <opencv2/opencv.hpp>

cv::Mat Preprocess::cropRight(const cv::Mat &image) const {
    // Check if the image is wide enough for cropping.
    if (image.cols <= crop_pixels_) {
        // Optionally, you might want to handle this case differently,
        // such as throwing an error. Here, we return a clone of the input.
        return image.clone();
    }

    // Define the region-of-interest: start at (0,0), include full height,
    // and reduce the width by crop_pixels_.
    cv::Rect roi(0, 0, image.cols - crop_pixels_, image.rows);

    // Crop the image and return a new copy.
    cv::Mat croppedImage = image(roi).clone();
    return croppedImage;
}
