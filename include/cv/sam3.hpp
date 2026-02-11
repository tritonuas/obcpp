#ifndef SAM3_HPP
#define SAM3_HPP

#include <onnxruntime_cxx_api.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <opencv2/opencv.hpp>

/**
 * @brief Structure representing a single object detection
 */
struct Detection {
    float x1, y1, x2, y2;  // Bounding box coordinates (Pixels)
    float confidence;      // Confidence score (0.0 - 1.0)
    int class_id;          // Class ID (usually 0 for text prompts)
    std::string label;     // Text label (e.g., "cat")
};

class SAM3 {
 public:
    /**
     * @brief Construct the SAM3 Detector with split encoder/decoder models
     * @param encoderPath Path to the encoder .onnx model (sam3_encoder_fp16.onnx)
     * @param decoderPath Path to the decoder .onnx model (sam3_decoder_fp16.onnx)
     * @param tokenizerPath Path to tokenizer.json
     * @param min_confidence threshold to filter weak detections (default 0.30)
     * @param nms_iou threshold for Non-Maximum Suppression (default 0.2)
     */
    SAM3(const std::string& encoderPath, const std::string& decoderPath,
         const std::string& tokenizerPath, double min_confidence = 0.30, double nms_iou = 0.2);

    /**
     * @brief Run inference on an image (draws boxes on the provided image)
     * @param image Input image (BGR - standard OpenCV format)
     * @param prompts Vector of text prompts to detect (run sequentially and aggregate)
     * @return std::vector<Detection> List of detected objects
     */
    std::vector<Detection> detect(cv::Mat& image, const std::vector<std::string>& prompts);

 private:
    // --- Helper Functions ---
    std::vector<int64_t> tokenize(const std::string& text);
    std::vector<float> preprocess(const cv::Mat& image);
    void loadTokenizer(const std::string& path);

    // --- Configuration ---
    double min_confidence_;
    double nms_iou_;

    // --- ONNX Runtime Resources ---
    std::shared_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::Session> encoderSession_;
    std::unique_ptr<Ort::Session> decoderSession_;
    Ort::SessionOptions sessionOptions_;

    // --- Tokenizer Resources ---
    std::unordered_map<std::string, int> vocab_;

    // CLIP Tokenizer Constants (Standard for SAM 3)
    int startToken_ = 49406;
    int endToken_ = 49407;
    int padToken_ = 49407;
    size_t contextLength_ = 32;  // or 128, must match your C++ implementation logic
};

#endif  // SAM3_HPP