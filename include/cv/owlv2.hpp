#ifndef INCLUDE_CV_OWLV2_HPP_
#define INCLUDE_CV_OWLV2_HPP_

#include <onnxruntime_cxx_api.h>

#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

// Reuse Detection struct
#include "cv/yolo.hpp"

class OWLv2 {
 public:
    // tokensPath is optional; if empty, we will try to locate a default
    explicit OWLv2(const std::string &modelPath, int inputSize = 960, float scoreThreshold = 0.10f,
                   float nmsIouThreshold = 0.50f, int maxDetections = 100,
                   const std::string &tokensPath = "");

    ~OWLv2();

    // Inference API compatible with YOLO wrapper
    std::vector<Detection> detect(const cv::Mat &bgrImage);
    void drawAndPrintDetections(cv::Mat &image, const std::vector<Detection> &detections);
    void processAndSaveImage(const cv::Mat &image, const std::string &outputFile);

 private:
    // Preprocess to RGB, letterbox to square (inputSize_ x inputSize_), normalize and CHW pack
    cv::Mat letterboxRGB(const cv::Mat &rgb, int targetLong, int &outH, int &outW, float &scale,
                         int &padTop, int &padLeft);

    std::vector<float> hwcToChwNormalize(const cv::Mat &rgbPadded) const;

    // Token loading helpers
    nlohmann::json loadTokensJson(const std::string &path) const;
    bool resolveTokens(const std::string &explicitPath, std::vector<int64_t> &ids,
                       std::vector<int64_t> &mask, int &Q, int &L) const;

    // NMS helper
    std::vector<int> nms(const std::vector<cv::Rect2f> &boxes, const std::vector<float> &scores,
                         float iouThresh, int maxKeep) const;

 private:
    // ORT
    Ort::Env env_;
    Ort::Session *session_;
    Ort::SessionOptions sessionOptions_;

    // Config
    std::string modelPath_;
    std::string tokensPath_;
    int inputSize_;
    float scoreThreshold_;
    float nmsIouThreshold_;
    int maxDetections_;

    // Preprocess bookkeeping
    float lastScale_ = 1.f;
    int lastPadTop_ = 0;
    int lastPadLeft_ = 0;
};

#endif  // INCLUDE_CV_OWLV2_HPP_
