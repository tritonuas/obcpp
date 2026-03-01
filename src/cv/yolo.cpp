#include "cv/yolo.hpp"

#include <algorithm>
#include <atomic>
#include <unordered_map>

// For simplicity, we are not doing advanced error handling
// Make sure to catch and handle exceptions in production code

namespace {
/**
 * @brief Computes the Intersection over Union (IoU) between two detection boxes.
 *
 * @param a First detection box.
 * @param b Second detection box.
 * @return float The IoU value in the range [0, 1].
 */
float computeIoU(const Detection& a, const Detection& b) {
    const float ax1 = std::max(a.x1, b.x1);
    const float ay1 = std::max(a.y1, b.y1);
    const float ax2 = std::min(a.x2, b.x2);
    const float ay2 = std::min(a.y2, b.y2);

    const float interW = std::max(0.0f, ax2 - ax1);
    const float interH = std::max(0.0f, ay2 - ay1);
    const float inter = interW * interH;

    const float areaA = std::max(0.0f, a.x2 - a.x1) * std::max(0.0f, a.y2 - a.y1);
    const float areaB = std::max(0.0f, b.x2 - b.x1) * std::max(0.0f, b.y2 - b.y1);
    const float uni = areaA + areaB - inter;
    if (uni <= 0.0f) return 0.0f;
    return inter / uni;
}

/**
 * @brief Performs Non-Maximum Suppression (NMS) on detections, independently for each class.
 *
 * Groups detections by class, sorts them by confidence, and suppresses boxes with
 * IoU greater than the threshold relative to higher-confidence boxes.
 * Basically this removes duplicate detections that are very close to one another.
 *
 * @param detections Vector of all detections to filter.
 * @param iouThreshold IoU threshold for suppression.
 * @return std::vector<Detection> Filtered list of detections.
 */
std::vector<Detection> nmsPerClass(const std::vector<Detection>& detections, float iouThreshold) {
    std::vector<Detection> kept;
    if (detections.empty()) return kept;

    std::unordered_map<int, std::vector<Detection>> byClass;
    // Reserve buckets to avoid rehashing.
    // (We only have 2 classes, but it's better to be safe than sorry.)
    byClass.reserve(16);
    for (const auto& d : detections) {
        byClass[d.class_id].push_back(d);
    }

    for (auto& kv : byClass) {
        std::vector<Detection>& dets = kv.second;
        std::sort(dets.begin(), dets.end(), [](const Detection& a, const Detection& b) {
            return a.confidence > b.confidence;
        });

        std::vector<char> suppressed(dets.size(), 0);
        for (size_t i = 0; i < dets.size(); ++i) {
            if (suppressed[i]) continue;
            kept.push_back(dets[i]);
            for (size_t j = i + 1; j < dets.size(); ++j) {
                if (suppressed[j]) continue;
                if (computeIoU(dets[i], dets[j]) > iouThreshold) {
                    suppressed[j] = 1;
                }
            }
        }
    }

    return kept;
}
}  // namespace

YOLO::YOLO(const std::string& modelPath, float confThreshold, int inputWidth, int inputHeight,
           float nmsThreshold)
    : env_(ORT_LOGGING_LEVEL_WARNING, "yolo-inference"),
      session_(nullptr),
      confThreshold_(confThreshold),
      inputWidth_(inputWidth),
      inputHeight_(inputHeight),
      nmsThreshold_(nmsThreshold) {
    // Initialize session options if needed
    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // Create the session
    try {
        session_ = new Ort::Session(env_, modelPath.c_str(), sessionOptions_);
        std::cout << "Model loaded successfully.\n";
    } catch (const Ort::Exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
    }

    // Create an allocator
    Ort::AllocatorWithDefaultOptions allocator;

    // Get input names
    size_t numInputNodes = session_->GetInputCount();
    inputNames_.resize(numInputNodes);
    for (size_t i = 0; i < numInputNodes; i++) {
        auto name = session_->GetInputNameAllocated(i, allocator);
        inputNames_[i] = std::string(name.get());
    }

    // Get output names
    size_t numOutputNodes = session_->GetOutputCount();
    outputNames_.resize(numOutputNodes);
    for (size_t i = 0; i < numOutputNodes; i++) {
        auto name = session_->GetOutputNameAllocated(i, allocator);  // function for output nodes
        outputNames_[i] = std::string(name.get());
    }
}

YOLO::~YOLO() {
    if (session_) {
        delete session_;
        session_ = nullptr;
    }
}

std::vector<float> YOLO::preprocess(const cv::Mat& image) {
    // Compute letterbox scale
    float r = std::min(static_cast<float>(inputWidth_) / static_cast<float>(image.cols),
                       static_cast<float>(inputHeight_) / static_cast<float>(image.rows));

    int unpadW = std::round(image.cols * r);
    int unpadH = std::round(image.rows * r);
    int dw = inputWidth_ - unpadW;   // total horizontal padding
    int dh = inputHeight_ - unpadH;  // total vertical padding

    // We'll store these so we can "un-letterbox" later:
    scale_ = r;
    padLeft_ = dw / 2;  // per-side left padding
    padTop_ = dh / 2;   // per-side top padding

    // Then call letterbox(...). This returns a new image of size (inputWidth_ x inputHeight_),
    // but we know how it was scaled & padded:
    // Note: 114 is the standard YOLO background padding color (grey)
    cv::Mat resized = letterbox(image, inputWidth_, inputHeight_, cv::Scalar(114, 114, 114));

    // Convert BGR->RGB if your model is trained on RGB:
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

    // Convert to float [0..1]
    resized.convertTo(resized, CV_32F, 1.0f / 255.0f);  // Normalize pixel values to [0, 1]

    // HWC -> CHW
    std::vector<float> inputData(inputWidth_ * inputHeight_ * 3);
    size_t index = 0;
    for (int c = 0; c < 3; c++) {
        for (int h = 0; h < inputHeight_; h++) {
            for (int w = 0; w < inputWidth_; w++) {
                inputData[index++] = resized.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    return inputData;
}

std::vector<Detection> YOLO::detect(const cv::Mat& image) {
    // Preprocess (letterbox to network size, store scale_ / padLeft_ / padTop_)
    std::vector<float> inputTensorValues = preprocess(image);

    // Create input tensor object from data values
    // 1 = batch size (we only process one image at a time)
    // 3 = number of channels (RGB)
    std::vector<int64_t> inputShape = {1, 3, static_cast<int64_t>(inputHeight_),
                                       static_cast<int64_t>(inputWidth_)};
    size_t inputTensorSize = static_cast<size_t>(1) * 3 * inputHeight_ * inputWidth_;

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorSize,
                                        inputShape.data(), inputShape.size());

    // Convert std::vector<std::string> to std::vector<const char*>
    std::vector<const char*> inputNamesCStr;
    inputNamesCStr.reserve(inputNames_.size());
    for (const auto& name : inputNames_) {
        inputNamesCStr.push_back(name.c_str());
    }

    std::vector<const char*> outputNamesCStr;
    outputNamesCStr.reserve(outputNames_.size());
    for (const auto& name : outputNames_) {
        outputNamesCStr.push_back(name.c_str());
    }

    // Run inference
    auto outputTensors =
        session_->Run(Ort::RunOptions{nullptr}, inputNamesCStr.data(), &inputTensor, 1,
                      outputNamesCStr.data(), outputNamesCStr.size());

    if (outputTensors.empty()) {
        std::cerr << "No output tensors from ONNX session.\n";
        return {};
    }

    Ort::Value& out = outputTensors[0];
    auto shapeInfo = out.GetTensorTypeAndShapeInfo();
    auto shape = shapeInfo.GetShape();

    // Expected: [1, C, N] or [1, N, C]
    if (shape.size() != 3) {
        std::cerr << "Unexpected output shape: ";
        for (auto d : shape) std::cerr << d << " ";
        std::cerr << std::endl;
        return {};
    }

    int64_t b = shape[0];
    int64_t d1 = shape[1];
    int64_t d2 = shape[2];

    if (b != 1) {
        std::cerr << "Only batch size 1 is supported, got batch = " << b << "\n";
        return {};
    }

    float* outputData = out.GetTensorMutableData<float>();
    std::vector<Detection> detections;

    // Helper lambdas for indexing and box/class extraction
    auto process_anchor_layout_CxN = [&](int64_t numChannels, int64_t numAnchors) {
        // Layout: [1, C, N], contiguous in row-major:
        // index = (c * numAnchors) + n
        for (int64_t a = 0; a < numAnchors; ++a) {
            auto get = [&](int64_t c) -> float { return outputData[c * numAnchors + a]; };

            float x = get(0);
            float y = get(1);
            float w = get(2);
            float h = get(3);

            // Find best class score and ID over channels 4..(numChannels-1)
            // Channels 0, 1, 2, 3 correspond to x, y, w, h respectively.
            // Class probabilities start at index 4.
            int bestClass = -1;
            float bestScore = 0.0f;
            for (int64_t c = 4; c < numChannels; ++c) {
                float s = get(c);
                if (s > bestScore) {
                    bestScore = s;
                    bestClass = static_cast<int>(c - 4);  // class index offset
                }
            }

            if (bestScore < confThreshold_ || bestClass < 0) {
                continue;
            }

            // Convert [x, y, w, h] (center in letterboxed input space) -> [x1, y1, x2, y2]
            float x1 = x - w * 0.5f;
            float y1 = y - h * 0.5f;
            float x2 = x + w * 0.5f;
            float y2 = y + h * 0.5f;

            // Remove letterbox padding
            x1 -= static_cast<float>(padLeft_);
            y1 -= static_cast<float>(padTop_);
            x2 -= static_cast<float>(padLeft_);
            y2 -= static_cast<float>(padTop_);

            // Scale back to original image coordinates
            if (scale_ > 0.0f) {
                x1 /= scale_;
                y1 /= scale_;
                x2 /= scale_;
                y2 /= scale_;
            }

            // Clamp to image boundaries
            float imgW = static_cast<float>(image.cols);
            float imgH = static_cast<float>(image.rows);

            x1 = std::max(0.0f, std::min(x1, imgW - 1.0f));
            y1 = std::max(0.0f, std::min(y1, imgH - 1.0f));
            x2 = std::max(0.0f, std::min(x2, imgW - 1.0f));
            y2 = std::max(0.0f, std::min(y2, imgH - 1.0f));

            Detection det;
            det.x1 = x1;
            det.y1 = y1;
            det.x2 = x2;
            det.y2 = y2;
            det.confidence = bestScore;
            det.class_id = bestClass;

            detections.push_back(det);
        }
    };

    auto process_anchor_layout_NxC = [&](int64_t numAnchors, int64_t numChannels) {
        // Layout: [1, N, C], contiguous:
        // for anchor a: row pointer = outputData + a * numChannels
        for (int64_t a = 0; a < numAnchors; ++a) {
            float* p = outputData + a * numChannels;

            float x = p[0];
            float y = p[1];
            float w = p[2];
            float h = p[3];

            int bestClass = -1;
            float bestScore = 0.0f;
            // Channels 0-3 are box coordinates, so iterate starting from 4 for class scores
            for (int64_t c = 4; c < numChannels; ++c) {
                float s = p[c];
                if (s > bestScore) {
                    bestScore = s;
                    bestClass = static_cast<int>(c - 4);
                }
            }

            if (bestScore < confThreshold_ || bestClass < 0) {
                continue;
            }

            float x1 = x - w * 0.5f;
            float y1 = y - h * 0.5f;
            float x2 = x + w * 0.5f;
            float y2 = y + h * 0.5f;

            // Remove letterbox padding
            x1 -= static_cast<float>(padLeft_);
            y1 -= static_cast<float>(padTop_);
            x2 -= static_cast<float>(padLeft_);
            y2 -= static_cast<float>(padTop_);

            // Scale back to original image size
            if (scale_ > 0.0f) {
                x1 /= scale_;
                y1 /= scale_;
                x2 /= scale_;
                y2 /= scale_;
            }

            float imgW = static_cast<float>(image.cols);
            float imgH = static_cast<float>(image.rows);

            x1 = std::max(0.0f, std::min(x1, imgW - 1.0f));
            y1 = std::max(0.0f, std::min(y1, imgH - 1.0f));
            x2 = std::max(0.0f, std::min(x2, imgW - 1.0f));
            y2 = std::max(0.0f, std::min(y2, imgH - 1.0f));

            Detection det;
            det.x1 = x1;
            det.y1 = y1;
            det.x2 = x2;
            det.y2 = y2;
            det.confidence = bestScore;
            det.class_id = bestClass;

            detections.push_back(det);
        }
    };

    // Choose layout based on which dimension looks like "channels"
    if (d1 < d2) {
        // [1, C, N]
        process_anchor_layout_CxN(d1, d2);
    } else {
        // [1, N, C]
        process_anchor_layout_NxC(d1, d2);
    }

    // Apply Non-Maximum Suppression per class
    detections = nmsPerClass(detections, nmsThreshold_);
    return detections;
}

cv::Mat YOLO::letterbox(const cv::Mat& src, int newWidth, int newHeight, const cv::Scalar& color) {
    float r = std::min(static_cast<float>(newWidth) / static_cast<float>(src.cols),
                       static_cast<float>(newHeight) / static_cast<float>(src.rows));

    int unpadW = std::round(src.cols * r);
    int unpadH = std::round(src.rows * r);

    cv::Mat dst;
    cv::resize(src, dst, cv::Size(unpadW, unpadH));

    int dw = newWidth - unpadW;
    int dh = newHeight - unpadH;

    int top = std::round(dh / 2.0f);
    int bottom = dh - top;
    int left = std::round(dw / 2.0f);
    int right = dw - left;

    cv::copyMakeBorder(dst, dst, top, bottom, left, right, cv::BORDER_CONSTANT, color);
    return dst;
}

void YOLO::drawAndPrintDetections(cv::Mat& image, const std::vector<Detection>& detections) {
    // Iterate through each detection and draw
    for (const auto& det : detections) {
        // Print detection info
        std::cout << "Detected class: " << det.class_id << " conf: " << det.confidence << " box: ["
                  << det.x1 << ", " << det.y1 << ", " << det.x2 << ", " << det.y2 << "]"
                  << std::endl;

        // Draw the bounding box
        cv::rectangle(image, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1)),
                      cv::Point(static_cast<int>(det.x2), static_cast<int>(det.y2)),
                      cv::Scalar(0, 255, 0),  // green box
                      2);                     // line thickness in pixels

        // Draw label (class + confidence) above the box
        std::string label = "ID:" + std::to_string(det.class_id) +
                            " conf:" + std::to_string(det.confidence).substr(0, 4);
        // Position label 5 pixels above top-left corner; Font scale 0.5, thickness 1
        cv::putText(image, label, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1) - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }
}

void YOLO::processAndSaveImage(const cv::Mat& image, const std::string& outputPath) {
    cv::Mat outputImage = image.clone();
    std::vector<Detection> detections = detect(outputImage);
    drawAndPrintDetections(outputImage, detections);

    // Generate a unique filename using a static atomic counter.
    static std::atomic<int> file_counter{0};
    int counter = file_counter.fetch_add(1);

    std::string uniqueOutputPath;
    // Find the position of the last dot and the last path separator
    size_t dotPos = outputPath.find_last_of('.');
    size_t sepPos = outputPath.find_last_of("/\\");  // supports both UNIX and Windows paths

    // If a dot exists after the last separator, assume it's a file extension.
    if (dotPos != std::string::npos && (sepPos == std::string::npos || dotPos > sepPos)) {
        uniqueOutputPath = outputPath.substr(0, dotPos) + "_" + std::to_string(counter) +
                           outputPath.substr(dotPos);
    } else {
        // No extension found; default to appending counter and .jpg extension.
        uniqueOutputPath = outputPath + "_" + std::to_string(counter) + ".jpg";
    }

    if (!cv::imwrite(uniqueOutputPath, outputImage)) {
        std::cerr << "Failed to write output image to " << uniqueOutputPath << std::endl;
    } else {
        std::cout << "Output saved to " << uniqueOutputPath << std::endl;
    }
}
