#ifndef INCLUDE_CV_YOLO_HPP_
#define INCLUDE_CV_YOLO_HPP_

#include <onnxruntime_cxx_api.h>

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

/// Simple struct to store a detection result
struct Detection {
    float x1;
    float y1;
    float x2;
    float y2;
    float confidence;
    int class_id;
};

class YOLO {
 public:
    /**
     * @brief Construct a new YOLO object
     *
     * @param modelPath Path to the YOLO .onnx file
     * @param confThreshold Minimum confidence threshold for a detection
     * @param inputWidth Width of the model input
     * @param inputHeight Height of the model input
     * @param nmsThreshold IoU threshold for Non-Maximum Suppression
     */
    YOLO(const std::string& modelPath, float confThreshold = 0.25f, int inputWidth = 1024,
         int inputHeight = 1024, float nmsThreshold = 0.9f);

    /**
     * @brief Destroy the YOLO object
     */
    ~YOLO();

    /**
     * @brief Perform inference on an input image.
     *
     * @param image Input image (cv::Mat in BGR format)
     * @return std::vector<Detection> A list of detections
     */
    std::vector<Detection> detect(const cv::Mat& image);

    /**
     * @brief Draws and prints the given detections on the image.
     *
     * @param image The original image on which boxes and labels will be drawn
     * @param detections The list of detections to visualize
     */
    void drawAndPrintDetections(cv::Mat& image, const std::vector<Detection>& detections);

    /**
     * @brief Process an input image: detect objects, draw detections, and save the output image.
     *
     * @param image The input image (cv::Mat in BGR format)
     * @param outputFile Path to save the output image.
     */
    void processAndSaveImage(const cv::Mat& image, const std::string& outputFile);

 private:
    /// Preprocess a cv::Mat to match the model's input shape and format
    std::vector<float> preprocess(const cv::Mat& image);

    /**
     * @brief Resize + pad the image to maintain aspect ratio as typical YOLO does.
     *
     * @param src       The original BGR image
     * @param newWidth  Target width (e.g. 640)
     * @param newHeight Target height (e.g. 640)
     * @param color     Letterbox color fill (default 114 for typical YOLO)
     * @return cv::Mat  The letterboxed image of size [newHeight x newWidth]
     */
    cv::Mat letterbox(const cv::Mat& src, int newWidth, int newHeight,
                      const cv::Scalar& color = cv::Scalar(114, 114, 114));

 private:
    Ort::Env env_;
    Ort::Session* session_;
    Ort::SessionOptions sessionOptions_;

    float confThreshold_;
    float nmsThreshold_;
    int inputWidth_;
    int inputHeight_;
    float scale_ = 1.f;
    int padTop_ = 0;
    int padLeft_ = 0;

    // Input/Output node information
    std::vector<std::string> inputNames_;
    std::vector<std::string> outputNames_;
};

#endif  // INCLUDE_CV_YOLO_HPP_
